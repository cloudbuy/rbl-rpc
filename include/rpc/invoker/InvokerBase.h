#ifndef RBL_RPC_INVOKER_BASE
#define RBL_RPC_INVOKER_BASE
namespace rubble { namespace rpc {
  class InvokerBase
  {
  public:
    InvokerBase()
    : m_client_data(new ClientData()) {}

    virtual bool is_useable() = 0;
    virtual ~InvokerBase(){};
    
    inline basic_protocol::ClientRequest &   request()
    {
      return m_client_data->request();
    }
    inline basic_protocol::ClientResponse &  repsonse()
    {
      return m_client_data->response();
    }
    inline ClientData::shptr & client_data()
    {
      return m_client_data;
    }
  protected:
    ClientData::shptr m_client_data;
  };
} }
#endif 
