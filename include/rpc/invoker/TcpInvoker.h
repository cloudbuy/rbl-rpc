#ifndef RBL_RPC_TCP_INVOKER_H
#define RBL_RPC_TCP_INVOKER_H
#include <rpc/invoker/InvokerBase.h>
#include <boost/asio.hpp>

namespace rubble { namespace rpc {
  class TcpInvokerException
  {
  };

  struct TcpInvoker : public InvokerBase
  {
    TcpInvoker(const std::string & str, short port)
      : m_io_service(),
        m_socket(m_io_service),
        m_endpoint(boost::asio::ip::address::from_string(str), port),
        m_useable(false)

    {
      m_socket.connect(m_endpoint, m_error_code);
      
      if(!m_error_code)
        m_useable = true;
    }

    void reset()
    {
      m_client_data->request().Clear();
      m_client_data->response().Clear();
      m_client_data->error_code().clear();
      BOOST_ASSERT_MSG( m_client_data->is_rpc_active() == false, 
        "THE FLAG THAT REPRESENTS ACTIVE "
        "RPC SHOULD NOT BE SET WHEN RESETING AN OBJECT FOR RPC");
    }

    void invoke()
    {
    }
   
    void operator() ()
    {
    }

    void after_post()
    {
    }

     bool is_useable()
    {
      return m_useable;
    }
  
    bool                            m_useable;
    boost::asio::io_service         m_io_service;
    boost::asio::ip::tcp::socket    m_socket;
    boost::asio::ip::tcp::endpoint  m_endpoint;
    boost::system::error_code       m_error_code; 
  };

} }
#endif 
