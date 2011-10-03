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

// struct TcpFrontEndConnectionInvoker ////////////////////////////////////////
  struct TcpFrontEndConnectionInvoker : public InvokerBase
  {
    typedef boost::shared_ptr<TcpFrontEndConnectionInvoker>  shptr;

     TcpFrontEndConnectionInvoker(SharedSocket s_in,TcpFrontEnd & tfe);

    ~TcpFrontEndConnectionInvoker();

    bool is_useable();
    void reset();
    void operator() ();
    void after_post();
    bool invoke();
    void handle_error(const boost::system::error_code & error, const char * method_name);
    void handle_read_header(  std::size_t bytes_sent, const boost::system::error_code & error);
    void handle_read_body(   std::size_t bytes_sent,
                            const boost::system::error_code & error);
    void handle_write_response();
    void handle_reset_for_next_request( std::size_t bytes_sent,
                                        const boost::system::error_code & error);

    SharedSocket                    socket;
    boost::shared_ptr<Buffer>       buffer;
    FRONT_END_CONNECTION_IO_STATE   io_state;
    TcpFrontEnd &                   tcp_front_end;
  };
//---------------------------------------------------------------------------//

// class TcpFrontEnd //////////////////////////////////////////////////////////
  class TcpFrontEnd 
  {
  public:
    typedef boost::scoped_ptr<TcpFrontEnd> scptr;
    
    TcpFrontEnd(BackEnd & b_in, short port);
    ~TcpFrontEnd();
  
    void connect_invoker_manager();
    void terminate_invoker_manager();
  
    void start();
    void stop();
    void join();
    
    boost::int32_t & rpc_count() { return m_rpc_count; }
    bool is_accepting() { return m_accepting_requests; }
    BackEnd & backend() { return m_backend; }

  private:
    void start_accept();
    void handle_accept(SharedSocket socket, const boost::system::error_code & error);
  

    
    boost::int32_t                                  m_rpc_count;
    bool                                            m_accepting_requests;
    boost::asio::io_service                         m_io_service;
    boost::asio::ip::tcp::acceptor                  m_acceptor;
    std::set<TcpFrontEndConnectionInvoker::shptr >  m_connections;
    boost::thread                                   m_thread;
    bool                                            m_started;
    BackEnd &                                       m_backend;
  };
//---------------------------------------------------------------------------//
} }

#endif 
