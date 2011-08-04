#ifndef RBL_RPC_SERVER_RPC_COMMON_H
#define RBL_RPC_SERVER_RPC_COMMON_H
#include <rpc/common/rpc_common.h>

namespace rubble { namespace rpc {
  class ClientCookieBase;
  class ClientData;

  class ServiceBase
  {
  public:
    typedef boost::shared_ptr<ServiceBase> shp;

    virtual void init(boost::system::error_code & ec) =0;
    virtual void teardown(boost::system::error_code & ec)=0;
    virtual void subscribe(ClientCookie & client_cookie, ClientData & cd,
      std::string *, std::string *) = 0;
    virtual void unsubscribe(ClientCookie & client_cookie, ClientData & cd) = 0;
    virtual bool contains_function_at_ordinal(boost::uint16_t ordinal) = 0;
    virtual void dispatch(ClientCookie & client_cookie, ClientData & cd)=0;
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
   
 
} } 

#include <rpc/proto/BasicProtocol-server.rblrpc.h>
#endif 
