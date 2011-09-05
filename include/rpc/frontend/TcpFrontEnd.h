#ifndef RBL_RPC_TCP_FRONT_END_H
#define RBL_RPC_TCP_FRONT_END_H
#include <rpc/invoker/InvokerBase.h>
#include <rpc/backend/BackEndBase.h>
#include <rpc/backend/ClientData.h>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <set>

namespace rubble {
namespace rpc {
class TcpFrontEnd;

typedef boost::shared_ptr<boost::asio::ip::tcp::socket> SharedSocket;

enum FRONT_END_CONNECTION_IO_STATE
{
  IO_READ_HEADER_WAIT_REQUEST_START_INACTIVE = 0,
  IO_READ_BODY_REQUEST_ACTIVE = 1,
  IO_REQUEST_DISPATCHED_ACTIVE = 2, 
  IO_REQUEST_RESPONSE_WRITE_ACTIVE =3
};

class FrontEndException
{
};

struct TcpFrontEndConnectionInvoker : public InvokerBase
{
  typedef boost::shared_ptr<TcpFrontEndConnectionInvoker>  shptr;

   TcpFrontEndConnectionInvoker(BackEnd & b, SharedSocket s_in,TcpFrontEnd & tfe)
    : socket(s_in),
      backend(b),
      io_state(IO_READ_HEADER_WAIT_REQUEST_START_INACTIVE),
      buffer(new Buffer),
      tcp_front_end(tfe)
  {
    backend.connect(m_client_data);
  }

  ~TcpFrontEndConnectionInvoker()
  {
    backend.disconect(m_client_data); 
  }
 
  bool is_useable()
  {
    return backend.is_useable();
  }

  void reset()        
  {  
    m_client_data->request().Clear();
    m_client_data->response().Clear();
    m_client_data->error_code().clear();
  }

  void after_post()   
  {
  }

  void operator() ()  
  {
    service->dispatch(*client_cookie, * m_client_data);
    backend.end_rpc(m_client_data.get());
    handle_write_response();  
  };

  void invoke()       
  {
    std::cout << "invoke: revise error handling " << this << std::endl; 
    response().set_error(basic_protocol::REQUEST_SUCCESS);
    backend.invoke(*this);
  };


  void handle_error(const boost::system::error_code & error, const char * method_name)
  {
    if(error.category() == boost::asio::error::get_misc_category() )
    {
      if(error.value() == boost::asio::error::eof)
      {
        std::cout << "client disconected" << std::endl;
      }
    }
    else if(error.category() == boost::asio::error::get_system_category() )
    {
      if(error.value() ==boost::asio::error::operation_aborted )
      {
        std::cout << "stop called:" << std::endl;
      }
    }
    else
    {
     std::cout << "error in " << method_name << ": " << error.category().name() 
      << " " << error.value() << " : "<< error.message() << std::endl;
    }
  }

  void handle_read_header(  std::size_t bytes_sent, const boost::system::error_code & error)
  {
    if(!error)
    {
      std::cout << "read header " << socket.use_count() << " this: "  << this << std::endl;

      boost::uint32_t msg_sz;
      google::protobuf::io::CodedInputStream cis(buffer->get(),8);
          
      cis.ReadLittleEndian32 ( & m_client_data->flags() );
      cis.ReadLittleEndian32 ( & msg_sz );

      if(msg_sz < buffer->size())
            buffer->resize(msg_sz);

      io_state = IO_READ_BODY_REQUEST_ACTIVE;
      boost::asio::async_read(  *socket.get(), 
                                    boost::asio::buffer( buffer->get(), ( msg_sz -8)),
                                    boost::bind(&TcpFrontEndConnectionInvoker::handle_read_body, 
                                      this, 
                                      boost::asio::placeholders::bytes_transferred,
                                      boost::asio::placeholders::error));
    }
    else handle_error(error, "handle_read_header");
  }

  void handle_read_body(   std::size_t bytes_sent,
                          const boost::system::error_code & error)
  {
    std::cout << "read body" << socket.use_count() << " this: "  << this << std::endl;

    if(!error)
    {
      m_client_data->request().ParseFromArray( buffer->get(), bytes_sent);
      io_state = IO_REQUEST_DISPATCHED_ACTIVE;

      invoke(); 
    }
    else handle_error(error,"handled_read_body");
  }

  void handle_write_response()
  {
    io_state = IO_REQUEST_RESPONSE_WRITE_ACTIVE;

    boost::uint32_t flag_return;
    boost::uint32_t msg_size_return;

    boost::uint32_t msg_size = 8 + response().ByteSize();
   
 
    if(msg_size > buffer->size() )
      buffer->resize(msg_size);

    google::protobuf::io::ArrayOutputStream aos(buffer->get(),buffer->size());
    google::protobuf::io::CodedOutputStream cos(&aos);
    
    cos.WriteLittleEndian32(0); // FOR RPC FLAGS -- UNUSED ATM
    cos.WriteLittleEndian32(msg_size);

    response().SerializeToCodedStream(&cos);
    
    std::cout << "write response: " << cos.ByteCount() << ":" << socket.use_count() << " this: "  << this << std::endl;
   
    boost::asio::async_write( *socket.get(),
      boost::asio::buffer(buffer->get(), msg_size),
      boost::bind ( &TcpFrontEndConnectionInvoker::handle_reset_for_next_request,
                    this,
                    boost::asio::placeholders::bytes_transferred,
                    boost::asio::placeholders::error)
    );
  }
  
  void handle_reset_for_next_request( std::size_t bytes_sent,
                                      const boost::system::error_code & error)
  {
    if(!error)
    {
    io_state = IO_READ_HEADER_WAIT_REQUEST_START_INACTIVE; 
    std::cout << "handle reset : " << socket.use_count() << " this: " << this<< std::endl;

    boost::asio::async_read(  *socket.get(),
                                  boost::asio::buffer ( buffer->get(), 8 ),
                                  boost::bind ( 
                                    &TcpFrontEndConnectionInvoker::handle_read_header, 
                                    this,
                                    boost::asio::placeholders::bytes_transferred,
                                    boost::asio::placeholders::error 
                                  ) 
                                );

    }
    else handle_error(error,"handle_reset_for_next_request");
  }

  SharedSocket                    socket;
  boost::shared_ptr<Buffer>       buffer;
  BackEnd &                       backend;
  FRONT_END_CONNECTION_IO_STATE   io_state;
  TcpFrontEnd &                   tcp_front_end;
};

  class TcpFrontEnd
  {
  public:
    typedef boost::scoped_ptr<TcpFrontEnd> scptr;
    
    TcpFrontEnd(BackEnd & b_in, short port)
      : m_io_service(),
        m_backend(b_in),
        m_acceptor( m_io_service, 
                    boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                    port)),
        m_started(false)
  {
  }

  ~TcpFrontEnd()
  {
    stop();
  }

  void start()
  {
    if( ! m_started )
    {
      if( ! m_backend.is_useable() )   
        throw FrontEndException();
      else
      {
        start_accept();
        m_thread = boost::thread( boost::bind(&boost::asio::io_service::run, &m_io_service) ); 
        std::cout << m_thread.get_id() << std::endl;
        m_started = true;
      }
    }
    else 
      throw FrontEndException();
  }

  void stop()
  {
    m_io_service.stop();
    join();
  } 
 
  void join()
  { 
    m_thread.join();
  }

private:
  void start_accept()
  {
    SharedSocket socket(new boost::asio::ip::tcp::socket(m_io_service) );

    m_acceptor.async_accept( *socket.get(), 
      boost::bind(&TcpFrontEnd::handle_accept, this, 
        socket,boost::asio::placeholders::error));
    
  }

  void handle_accept(SharedSocket socket, const boost::system::error_code & error)
  {
    if(!error)
    { 
      TcpFrontEndConnectionInvoker::shptr connection(
        new TcpFrontEndConnectionInvoker( m_backend, socket, *this));
       
      m_connections.insert(connection);

      boost::asio::async_read(  *connection->socket.get(),
                                  boost::asio::buffer ( connection->buffer->get(), 8 ),
                                  boost::bind ( 
                                    &TcpFrontEndConnectionInvoker::handle_read_header, 
                                    connection,
                                    boost::asio::placeholders::bytes_transferred,
                                    boost::asio::placeholders::error 
                                  ) 
                                );
    }
    else
    {
      std::cout << "error in handle_accept : val(" << error.value() << ") - str(" << error.message()<< ")."<< std::endl;
    }
    start_accept();
  }

  

  BackEnd &                                       m_backend;
  boost::asio::io_service                         m_io_service;
  boost::asio::ip::tcp::acceptor                  m_acceptor;
  std::set<TcpFrontEndConnectionInvoker::shptr >  m_connections;
  boost::thread                                   m_thread;
  bool                                            m_started;
};

} }

#endif 
