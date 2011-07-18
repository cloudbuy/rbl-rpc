#ifndef RBL_RPC_SERVER_RPC_COMMON_H
#define RBL_RPC_SERVER_RPC_COMMON_H

#include "rpc_common.h"

namespace rubble { namespace rpc {

  class ServiceBase
  {
  public:
    virtual bool Init() =0;
    virtual bool TearDown()=0;
    virtual bool Dispatch( basic_protocol::ClientRequest & cr)=0;
    virtual const char * name() = 0;
    virtual ~ServiceBase(){};
  };
} } 

#include <rpc/proto/BasicProtocol-server.rblrpc.h>

namespace rubble { namespace rpc {
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
        service_count(0) 
  {
  }
  
  bool RegisterService(ServiceBase_shp service)
    {
      common::OP_RESPONSE ret = m_services.SetEntry(
        common::Oid(service->name(),service_count),service);
      if(ret == common::OP_NO_ERROR)
        return true; 
      else 
        return false;
    }
   
  private:
    common::OidContainer<common::Oid, ServiceBase_shp> m_services;
    boost::uint8_t service_count;
  };

} }

#endif 
