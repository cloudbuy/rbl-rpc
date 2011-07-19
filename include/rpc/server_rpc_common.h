#ifndef RBL_RPC_SERVER_RPC_COMMON_H
#define RBL_RPC_SERVER_RPC_COMMON_H

#include "rpc_common.h"

namespace rubble { namespace rpc {

  class ServiceBase
  {
  public:
    virtual bool Init() =0;
    virtual bool TearDown()=0;
    virtual bool Dispatch( void * client_cookie, basic_protocol::ClientRequest & cr)=0;
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
  class ClientData
  {
  public:
    ClientData() 
      : m_flags(0),
        m_buffer_size(RBL_RPC_CONF_INITIAL_BUFFER_SIZE),
        m_buffer(new boost::uint8_t[RBL_RPC_CONF_INITIAL_BUFFER_SIZE]) {}

    boost::uint8_t * resize_buffer(std::size_t new_size)
    {
      m_buffer.reset(new boost::uint8_t[new_size]);
      m_buffer_size=new_size;
      return m_buffer.get();
    }
    
    boost::uint8_t * buffer() { return m_buffer.get(); } 
    boost::uint32_t & flags() { return m_flags; }
    basic_protocol::ClientRequest request() { return m_request; }

    std::size_t buffer_size() { return m_buffer_size; }
  private:
    basic_protocol::ClientRequest m_request;
    std::string name;
   
    std::size_t m_buffer_size;
    boost::scoped_array<boost::uint8_t> m_buffer;
    boost::uint32_t m_flags;
  };

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
