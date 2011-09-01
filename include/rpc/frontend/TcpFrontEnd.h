#ifndef RBL_RPC_TCP_FRONT_END_H
#define RBL_RPC_TCP_FRONT_END_H
#include <rpc/invoker/InvokerBase.h>
#include <rpc/backend/BackEndBase.h>
#include <rpc/backend/ClientData.h>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <set>

namespace rubble {
namespace rpc {

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
  };

  void invoke()       
  {
    backend.invoke(*this);
  };

  TcpFrontEndConnectionInvoker(BackEnd & b, SharedSocket s_in)
    : socket(s_in),
      backend(b),
      io_state(IO_READ_HEADER_WAIT_REQUEST_START_INACTIVE)
  {
    
  }

  void handle_read_header(  std::size_t bytes_sent, const boost::system::error_code & error)
  {
    if(!error)
    {
      boost::uint32_t msg_sz;
      google::protobuf::io::CodedInputStream cis(buffer.get(),8);
          
      cis.ReadLittleEndian32 ( & m_client_data->flags() );
      cis.ReadLittleEndian32 ( & msg_sz );

      if(msg_sz < buffer.size())
            buffer.resize(msg_sz);

      io_state = IO_READ_BODY_REQUEST_ACTIVE;

      boost::asio::async_read(  *socket.get(), 
                                    boost::asio::buffer( buffer.get(), ( msg_sz -8)),
                                    boost::bind(&TcpFrontEndConnectionInvoker::handle_read_body, 
                                      this, 
                                      boost::asio::placeholders::bytes_transferred,
                                      boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "error in handle_read_header" << std::endl;
    }
  }

  void handle_read_body(   std::size_t bytes_sent,
                          const boost::system::error_code & error)
  {
    if(!error)
    {
      m_client_data->request().ParseFromArray( buffer.get(), bytes_sent);
      io_state = IO_REQUEST_DISPATCHED_ACTIVE;
    }
    else
    {
      std::cout <<"error in handle_ready_body" << std::endl;
    }
  }

  SharedSocket                    socket;
  Buffer                          buffer;
  BackEnd &                       backend;
  FRONT_END_CONNECTION_IO_STATE   io_state;
};

class TcpFrontEnd
{
public:
  TcpFrontEnd(BackEnd & b_in, short port)
    : m_io_service(),
      m_backend(b_in),
      m_acceptor( m_io_service, 
                  boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                  port)),
      m_started(false)
  {
  }

  void start()
  {
    if( ! m_started )
    {
      if( ! m_backend.is_useable() )   
        throw FrontEndException();
      else
        start_accept();
    }
    else 
      throw FrontEndException();
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
    
//    m_thread = boost::thread( boost::bind(&boost::asio::io_service::run, &m_io_service) ); 
  }

  void handle_accept(SharedSocket socket, const boost::system::error_code & error)
  {
    std::cout << "accepted" << std::endl;
    if(!error)
    {
      TcpFrontEndConnectionInvoker::shptr connection(
        new TcpFrontEndConnectionInvoker( m_backend, socket));
       
      m_connections.insert(connection);

      boost::asio::async_read(  *connection->socket.get(),
                                  boost::asio::buffer ( &connection->buffer, 8 ),
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
      std::cout << "error in handle_accept : " << error.value()<< std::endl;
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
