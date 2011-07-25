#ifndef RBL_RPC_SERVER_RPC_COMMON_H
#define RBL_RPC_SERVER_RPC_COMMON_H
#include <rpc/common/rpc_common.h>

namespace rubble { namespace rpc {
  class ClientCookieBase;
  class ClientData;

  class ServiceBase
  {
  public:
    virtual bool Init() =0;
    virtual bool TearDown()=0;
    virtual bool Dispatch(ClientCookieBase ** client_cookie, ClientData * cd, basic_protocol::ClientRequest & cr)=0;
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

  class ServiceOracle 
  {
  public:
    ServiceOracle()
      : m_services(),
        m_is_sealed(false),
        m_service_count(0) 
    {
    }
  
    bool RegisterService(ServiceBase_shp service)
    {
      if(!m_is_sealed)
      {
        common::OP_RESPONSE ret = m_services.SetEntry(
          common::Oid(service->name(),m_service_count),service);
        if(ret == common::OP_NO_ERROR)
        {
          m_service_count++; 
          return true; 
        }
        else 
          throw "Service ordinal or name, allready exists;";
      }
        else throw "Service Oracle is final/sealed, cannot add more entries";
    }
    void seal()
    {
      
      m_is_sealed = true;
    }
  private:
    common::OidContainer<common::Oid, ServiceBase_shp> m_services;
    boost::uint8_t m_service_count;
    bool m_is_sealed;
  };

  
} } 

#include <rpc/proto/BasicProtocol-server.rblrpc.h>
#endif 
