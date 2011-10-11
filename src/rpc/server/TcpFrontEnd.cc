#include <rpc/frontend/TcpFrontEnd.h>

namespace rubble {
namespace rpc {
// class TcpFrontEndConnectionInvoker /////////////////////////////////////////
  // TcpFrontEndConnectionInvoker /////////////////////////////////////////////
  TcpFrontEndConnectionInvoker::TcpFrontEndConnectionInvoker(
    SharedSocket s_in,TcpFrontEnd & tfe)
    : socket(s_in),
      io_state(IO_READ_HEADER_WAIT_REQUEST_START_INACTIVE),
      buffer(new Buffer),
      tcp_front_end(tfe)
  {
    tcp_front_end.backend().connect(m_client_data);
  }
  //-------------------------------------------------------------------------//

  // ~TcpFrontEndConnectionInvoker ////////////////////////////////////////////
  TcpFrontEndConnectionInvoker::~TcpFrontEndConnectionInvoker()
  {
    tcp_front_end.backend().disconect(m_client_data); 
  }
  //-------------------------------------------------------------------------//

  // is_useable ///////////////////////////////////////////////////////////////
  bool TcpFrontEndConnectionInvoker::is_useable()
  {
    return tcp_front_end.backend().is_useable();
  }
  //-------------------------------------------------------------------------//  

  // reset ////////////////////////////////////////////////////////////////////
  void TcpFrontEndConnectionInvoker::reset()
  {  
    m_client_data->request().Clear();
    m_client_data->response().Clear();
    m_client_data->error_code().clear();
  }
  //-------------------------------------------------------------------------//

  // operator() ///////////////////////////////////////////////////////////////
  void TcpFrontEndConnectionInvoker::operator() ()  
  {
    service->dispatch(*client_cookie, * m_client_data);
    tcp_front_end.backend().end_rpc(m_client_data.get());
    handle_write_response();  
  };
  //-------------------------------------------------------------------------//

  // after_post ///////////////////////////////////////////////////////////////
  void TcpFrontEndConnectionInvoker::after_post()   
  {
  }
  /////////////////////////////////////////////////////////////////////////////

  // invoke ///////////////////////////////////////////////////////////////////
  bool TcpFrontEndConnectionInvoker::invoke()       
  {
    return tcp_front_end.backend().invoke(*this);
  };
  //-------------------------------------------------------------------------//
  
  // handle_error /////////////////////////////////////////////////////////////
  void TcpFrontEndConnectionInvoker::handle_error(
    const boost::system::error_code & error, const char * method_name )
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
  //-------------------------------------------------------------------------//

  // handle_read_heeader //////////////////////////////////////////////////////
  void TcpFrontEndConnectionInvoker::handle_read_header(  
    std::size_t bytes_sent, const boost::system::error_code & error)
  {
 
    if(!error)
    {
      boost::uint32_t msg_sz;
      google::protobuf::io::CodedInputStream cis(buffer->get(),8);
          
      cis.ReadLittleEndian32 ( & m_client_data->flags() );
      cis.ReadLittleEndian32 ( & msg_sz );

      if(msg_sz < buffer->size())
            buffer->resize(msg_sz);

      io_state = IO_READ_BODY_REQUEST_ACTIVE;
      boost::asio::async_read(  
        *socket.get(), 
        boost::asio::buffer( buffer->get(), ( msg_sz -8)),
        boost::bind(&TcpFrontEndConnectionInvoker::handle_read_body, 
          this, 
          boost::asio::placeholders::bytes_transferred,
          boost::asio::placeholders::error
        )
      );
    }
    else handle_error(error, "handle_read_header");
  }
  //-------------------------------------------------------------------------//

  // handle_read_body /////////////////////////////////////////////////////////
  void TcpFrontEndConnectionInvoker::handle_read_body(
    std::size_t bytes_sent, const boost::system::error_code & error)
  {

    if(!error)
    {
      m_client_data->request().ParseFromArray( buffer->get(), bytes_sent);
      io_state = IO_REQUEST_DISPATCHED_ACTIVE;

      if( invoke() )
        handle_write_response();
    }
    else handle_error(error,"handled_read_body");
  }
  //-------------------------------------------------------------------------//
  
  // handle_write_response ////////////////////////////////////////////////////
  void TcpFrontEndConnectionInvoker::handle_write_response()
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
    
    boost::asio::async_write( *socket.get(),
      boost::asio::buffer(buffer->get(), msg_size),
      boost::bind ( &TcpFrontEndConnectionInvoker::handle_reset_for_next_request,
                    this,
                    boost::asio::placeholders::bytes_transferred,
                    boost::asio::placeholders::error)
    );
  }
  //-------------------------------------------------------------------------//

  // handle_reset_for_next_request ////////////////////////////////////////////
  void TcpFrontEndConnectionInvoker::handle_reset_for_next_request
    ( std::size_t bytes_sent, const boost::system::error_code & error)
  {
    if(!error)
    {
      io_state = IO_READ_HEADER_WAIT_REQUEST_START_INACTIVE; 

      boost::asio::async_read(  
        *socket.get(),
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
  //-------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
 
// class TcpFrontEnd //////////////////////////////////////////////////////////
  // TcpFrontEnd //////////////////////////////////////////////////////////////
  TcpFrontEnd::TcpFrontEnd(BackEnd & b_in, short port)
    : m_io_service(),
      m_backend(b_in),
      m_started(false),
      m_rpc_count(0),
      m_port(port),
      m_accepting_requests(false),
      m_connected_to_backend(false)
  {
    if( ! m_backend.is_useable() )   
      throw FrontEndException();
    
    m_sig_connection_aptr = m_backend.register_invoker_manager(*this);
  }
  //-------------------------------------------------------------------------//

  // ~TcpFrontend /////////////////////////////////////////////////////////////
  TcpFrontEnd::~TcpFrontEnd()
  {
    stop();
  }
  //-------------------------------------------------------------------------//
  
  // stop /////////////////////////////////////////////////////////////////////
  //
  // things that need to happen for a stop.
  // 1. check if stop has not allready been performed.
  void TcpFrontEnd::stop()
  {
    m_io_service.stop();
    join();
  } 
  //-------------------------------------------------------------------------//

  // start ////////////////////////////////////////////////////////////////////
  void TcpFrontEnd::start()
  {
    if(m_started == true)
      throw FrontEndException();
    else
    {
      m_acceptor_scptr.reset(  new boost::asio::ip::tcp::acceptor( m_io_service, 
                    boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                    m_port)));

      start_accept();
      m_thread = boost::thread( boost::bind(  &boost::asio::io_service::run,
                                                  &m_io_service) ); 
      m_started = true;
    }
  }
  //-------------------------------------------------------------------------//

  
  // join /////////////////////////////////////////////////////////////////////
  void TcpFrontEnd::join()
  { 
    m_thread.join();
  }
  //-------------------------------------------------------------------------//
  
  // connect_to_backend /////////////////////////////////////////////////////// 
  void TcpFrontEnd::connect_to_backend()
  {
    if(m_started)
      throw FrontEndException();

    m_connected_to_backend = true;
  }
  //-------------------------------------------------------------------------//

  // disconect_from_backend ///////////////////////////////////////////////////
  void TcpFrontEnd::disconect_from_backend()
  {
    m_io_service.stop(); 
  }
  //-------------------------------------------------------------------------//

  // handle_accept ////////////////////////////////////////////////////////////
  void TcpFrontEnd::handle_accept(  SharedSocket socket, 
                                    const boost::system::error_code & error)
  {
    if(!error)
    { 
      TcpFrontEndConnectionInvoker::shptr connection(
        new TcpFrontEndConnectionInvoker( socket, *this));
       
      m_connections.insert(connection);

      boost::asio::async_read(  
        *connection->socket.get(),
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
      std::cout << "error in handle_accept : val(" << error.value()
        << ") - str(" << error.message()<< ")."<< std::endl;
    }
    start_accept();
  }
  //-------------------------------------------------------------------------//
  
  // start_accept /////////////////////////////////////////////////////////////
  void TcpFrontEnd::start_accept()
  {
    SharedSocket socket(new boost::asio::ip::tcp::socket(m_io_service) );
    m_accepting_requests = true; 

    m_acceptor_scptr->async_accept( *socket.get(), 
      boost::bind(&TcpFrontEnd::handle_accept, this, 
        socket,boost::asio::placeholders::error));
    
  }
  //-------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
} }
