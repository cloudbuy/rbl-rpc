#ifndef RBL_RPC_BACKEND
#define RBL_RPC_BACKEND
#include <rpc/common/rpc_errors.h>
#include <rpc/server/ClientData.h>
#include <rpc/server/ClientServiceCookies.h>
#include <rpc/server/ServiceBase.h>
#include <rpc/proto/BasicProtocol-server.rblrpc.h>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <set>
#include <iostream>


namespace rubble { namespace rpc {
  class BackEndBase
  {
  public:
    typedef common::OidContainer<common::Oid, ServiceBase::shp> t_services;

    BackEndBase(  basic_protocol::SourceConnectionType       source_type,
                  basic_protocol::DestinationConnectionType backend_type);

    void pool_size(int pool_size_in) { m_pool_size = pool_size_in; }
    bool is_sealed() { return m_is_sealed; } 
    void seal() { m_is_sealed = true;}
   
    void start();
    void register_and_init_service(ServiceBase::shp service);
    void block_till_termination();
    bool shutdown();
  
 
    void connect(ClientData::shp & client_data);
    void disconect(ClientData::shp & client_data);
    
    basic_protocol::SourceConnectionType source_type() const
      { return m_source_type;}
    basic_protocol::DestinationConnectionType destination_type() const
      { return m_backend_type; }
    const ClientServiceCookies & cookies() const 
      { return m_client_service_cookies; }
  protected:
    friend class BasicProtocolImpl;

    t_services & services() { return m_services;}
    ClientServiceCookies & cookies() { return m_client_service_cookies; }
    boost::recursive_mutex & mutex() { return m_mutex; }

    basic_protocol::SourceConnectionType                m_source_type;
    basic_protocol::DestinationConnectionType           m_backend_type;

    int                                                 m_pool_size;
    boost::thread_group                                 m_thread_group;
    boost::system::error_code                           m_ec;
    ClientServiceCookies                                m_client_service_cookies;
    std::set<ClientData::shp>                           m_connected_clients;    
 
    common::OidContainer<common::Oid, ServiceBase::shp> m_services;
    boost::uint16_t                                     m_service_count;
    bool                                                m_is_sealed;
 
    boost::asio::io_service                             m_io_service;
    boost::asio::io_service::work                       m_work;
    boost::recursive_mutex                              m_mutex;
  };

  class BasicProtocolImpl
  {
  public:
    typedef ClientCookieBase t_client_cookie;

    void init ( boost::system::error_code & ec);
    void teardown ( boost::system::error_code & ec);
 
    void hello (
        ClientCookie & cc, ClientData & cd,
        basic_protocol::HelloRequest & hr, basic_protocol::HelloResponse & hres);
    
    void list_services (  
        ClientCookie & cc ,ClientData & cd,
        basic_protocol::ListServicesRequest & req, 
        basic_protocol::ListServicesResponse & res);

    void subscribe (
        ClientCookie & client_cookie, ClientData & cd,
        std::string *,std::string *); 
    
    void unsubscribe ( ClientCookie & client_cookie, ClientData & cd);

    void rpc_subscribe_service ( 
        ClientCookie & cc,ClientData & cd,
        basic_protocol::SubscribeServiceRequest & req,
        basic_protocol::SubscribeServiceResponse & res);

    void rpc_unsubscribe_service ( 
        ClientCookie & cc,ClientData & cd,
        basic_protocol::UnsubscribeServiceRequest & req,
        basic_protocol::UnsubscribeServiceResponse & res);
   
    void list_methods (
      ClientCookie & cc,   ClientData & cd ,
      basic_protocol::ListMethodsRequest & req ,
      basic_protocol::ListMethodsResponse & res );
   
 
     void backend ( BackEndBase * backend );
  private:
     BackEndBase * m_backend;
  };

} }
#endif
