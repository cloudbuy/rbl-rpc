#ifndef RBL_RPC_BACKEND
#define RBL_RPC_BACKEND
#include <rpc/common/rpc_errors.h>
#include <rpc/server/ClientData.h>
#include <rpc/server/ClientServiceCookies.h>
#include <rpc/server/ServiceOracle.h>
#include <rpc/proto/BasicProtocol-server.rblrpc.h>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <set>
#include <iostream>


namespace rubble { namespace rpc {
  class BackEndBase;

  class BasicProtocolImpl
  {
  public:
    typedef ClientCookieBase t_client_cookie;

    void init(boost::system::error_code & ec) {ec.clear();}
    void teardown(boost::system::error_code & ec) {ec.clear();}
    void Hello(ClientCookie & cc,ClientData & cd,
      basic_protocol::HelloRequest & hr, basic_protocol::HelloResponse & hres)
      { std::cout << hr.node_name() << std::endl;}
    void ListServices(  ClientCookie &,ClientData &,
      basic_protocol::ListServicesRequest & , basic_protocol::ListServicesResponse & ){}
  
    void backend(BackEndBase * backend)
      { m_backend = backend; }
  private:
     BackEndBase * m_backend;
  };
  

  class BackEndBase
  {
  public:
    BackEndBase(  basic_protocol::SourceConnectionType       source_type
                  ,basic_protocol::DestinationConnectionType backend_type);

    void pool_size(int pool_size_in) { m_pool_size = pool_size_in; }
    bool is_sealed() { return m_is_sealed; } 
    void seal() { m_is_sealed = true;}
   
    void start();
    void register_and_init_service(ServiceBase_shp service);
    void block_till_termination();
    bool shutdown();
   
    void connect(ClientData::shp & client_data);
    void disconect(ClientData::shp & client_data); 
  protected:
    basic_protocol::SourceConnectionType                m_source_type;
    basic_protocol::DestinationConnectionType           m_backend_type;
    int                                                 m_pool_size;
    boost::thread_group                                 m_thread_group;
    boost::system::error_code                           m_ec;
    ClientServiceCookies                                m_client_service_cookies;
    std::set<ClientData::shp>                           m_connected_clients;    
 
    common::OidContainer<common::Oid, ServiceBase_shp>  m_services;
    boost::uint16_t                                     m_service_count;
    bool                                                m_is_sealed;
 
    boost::asio::io_service                             m_io_service;
    boost::asio::io_service::work                       m_work;
    boost::shared_mutex                                 m_mutex;
  };

} }
#endif
