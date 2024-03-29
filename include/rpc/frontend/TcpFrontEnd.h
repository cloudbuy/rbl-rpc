#ifndef RBL_RPC_TCP_FRONT_END_H
#define RBL_RPC_TCP_FRONT_END_H
#include <rpc/backend/BackEndBase.h>
#include <rpc/backend/ClientData.h>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <set>
#include <rpc/invoker/InProcessInvoker.h>

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

// struct TcpFrontEndConnectionInvoker /////////////////////////////////////////
  struct TcpFrontEndConnectionInvoker 
    : public InvokerBase, 
      public boost::enable_shared_from_this<TcpFrontEndConnectionInvoker>
  {
    typedef boost::shared_ptr<TcpFrontEndConnectionInvoker>  shptr;

     TcpFrontEndConnectionInvoker(SharedSocket s_in,TcpFrontEnd & tfe);

    ~TcpFrontEndConnectionInvoker();

    bool is_useable();
    void reset();
    void operator() ();
    void after_post();
    bool invoke();
    void handle_error(  const boost::system::error_code & error, 
                        const char * method_name);
    void handle_read_header(  std::size_t bytes_sent, 
                              const boost::system::error_code & error);

    void handle_read_body(    std::size_t bytes_sent,
                              const boost::system::error_code & error);
    void handle_write_response();
    void handle_reset_for_next_request( std::size_t bytes_sent,
                                        const boost::system::error_code &);

    SharedSocket                    socket;
    boost::shared_ptr<Buffer>       buffer;
    FRONT_END_CONNECTION_IO_STATE   io_state;
    TcpFrontEnd &                   tcp_front_end;  
  };
//----------------------------------------------------------------------------//
// class TcpFrontEnd ///////////////////////////////////////////////////////////
  class TcpFrontEnd 
  {
  public:
    typedef boost::scoped_ptr<TcpFrontEnd>                    scptr;
    typedef boost::scoped_ptr<boost::asio::ip::tcp::acceptor> acceptor_scptr;
    typedef std::set<TcpFrontEndConnectionInvoker::shptr >    t_connection_set;
    
  
    TcpFrontEnd(BackEnd & b_in, short port);
    ~TcpFrontEnd();
 
    // locked for outside of event loop / external usage. //
    void start();
    void stop();

    void connect_to_backend();
    void disconnect_from_backend();
    //----------------------------------------------------//

    // only accessed from event loop. /////////////////////////////////////////
    void disconnect_client(TcpFrontEndConnectionInvoker::shptr);

    boost::int32_t & rpc_count()    { return m_rpc_count;           }
    bool is_accepting()             { return m_accepting_requests;  }
    BackEnd & backend()             { return m_backend;             }
    std::size_t connection_count()  { return m_connections.size();  }
    bool & shutdown_initiated()     { return m_shutdown_initiated;  }
    //-----------------------------------------------------------------------//
     
  private:
    void shutdown_handler();

    void start_accept();
    void handle_accept( SharedSocket socket, 
                        const boost::system::error_code & error);
  
    
    boost::int32_t                                  m_rpc_count;
    short                                           m_port;
    bool                                            m_accepting_requests;
    boost::asio::io_service                         m_io_service;
    acceptor_scptr                                  m_acceptor_scptr;
    t_connection_set                                m_connections;
    boost::thread                                   m_thread;
    bool                                            m_started;
    bool                                            m_connected_to_backend;
    BackEnd &                                       m_backend;

    bool                                            m_shutdown_initiated;
    SynchronisedSignalConnection::aptr              m_sig_connection_aptr;
    NotificationObject                              m_shutdown_notification;
    boost::system::error_code                       m_error_code;
    
    boost::mutex                                    m_mutex; 
  };
//----------------------------------------------------------------------------//
} }

#endif 
