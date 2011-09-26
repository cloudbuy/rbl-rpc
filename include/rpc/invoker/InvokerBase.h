#ifndef RBL_RPC_INVOKER_BASE
#define RBL_RPC_INVOKER_BASE
#include <boost/shared_ptr.hpp>
#include <boost/signals/connection.hpp>

namespace rubble { namespace rpc {
  class BackEnd;

  struct InvokerBase
  {
    InvokerBase()
    : m_client_data(new ClientData()) {}
    virtual ~InvokerBase(){};

    virtual bool is_useable() = 0;
    virtual void reset() = 0;
    virtual void after_post() =0; 
    virtual void operator() () = 0;   
    virtual bool invoke() = 0;

    

    inline basic_protocol::ClientRequest &   request()
    {
      return m_client_data->request();
    }

    inline basic_protocol::ClientResponse &  response()
    {
      return m_client_data->response();
    }

    inline ClientData::shptr & client_data()
    {
      return m_client_data;
    }
  
    // this shared ptr below is used to determine when the last Invoker is 
    // destroyed. Any class deriving from this one should test if the pointer
    // ".unique()" and then destroy its members.
    ClientData::shptr m_client_data;
    ClientCookie::ptr client_cookie;
    ServiceBase::ptr service;
    boost::signals::connection m_sig_connection;
  };
} }
#endif 
