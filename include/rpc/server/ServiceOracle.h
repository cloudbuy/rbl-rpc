#ifndef RBL_RPC_SERVER_RPC_COMMON_H
#define RBL_RPC_SERVER_RPC_COMMON_H
#include <rpc/common/rpc_common.h>

namespace rubble { namespace rpc {
  class ClientCookieBase;
  class ClientData;

  class ServiceBase
  {
  public:
    virtual void init(boost::system::error_code & ec) =0;
    virtual void teardown(boost::system::error_code & ec)=0;
    virtual bool dispatch(ClientCookie * client_cookie, ClientData & cd, basic_protocol::ClientRequest & cr)=0;
    virtual const char * name() = 0;
    virtual ~ServiceBase(){};
    
    common::ordinal_type ordinal() { return m_ordinal; }
    void set_ordinal(common::ordinal_type ordinal) { m_ordinal = ordinal; }
  private:
    common::ordinal_type m_ordinal;
  };
/*
  class BasicProtocol
  {
  public:
    bool Init() {}
    bool TearDown() {}
    bool Hello(HelloRequest,HelloResponse){};
    bool ListServices(ListServicesRequest,ListServicesResponse){};
  private:
  };
*/
   typedef boost::shared_ptr<ServiceBase> ServiceBase_shp;

 
} } 

#include <rpc/proto/BasicProtocol-server.rblrpc.h>
#endif 
