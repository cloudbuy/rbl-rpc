#ifndef RBL_RPC_INVOKER_BASE
#define RBL_RPC_INVOKER_BASE
namespace rubble { namespace rpc {
  struct InvokerBase : public boost::noncopyable
  {
    InvokerBase()
    : m_client_data(new ClientData()) {}
    virtual ~InvokerBase(){};

    virtual bool is_useable() = 0;
    virtual void reset() = 0;
    virtual void after_post() =0; 
    virtual void operator() () = 0;   
    virtual void invoke() = 0;

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
  };
} }
#endif 
