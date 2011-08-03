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

    BackEndBase(  basic_protocol::SourceConnectionType       source_type
                  ,basic_protocol::DestinationConnectionType backend_type);

    void pool_size(int pool_size_in) { m_pool_size = pool_size_in; }
    bool is_sealed() { return m_is_sealed; } 
    void seal() { m_is_sealed = true;}
   
    void start();
    void register_and_init_service(ServiceBase::shp service);
    void block_till_termination();
    bool shutdown();
  
    const t_services services() const { return m_services;}
 
    void connect(ClientData::shp & client_data);
    void disconect(ClientData::shp & client_data);
    
    basic_protocol::SourceConnectionType source_type() const
      { return m_source_type;}
    basic_protocol::DestinationConnectionType destination_type() const
      { return m_backend_type; }
  protected:
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
    boost::shared_mutex                                 m_mutex;
  };

  class BasicProtocolImpl
  {
  public:
    typedef ClientCookieBase t_client_cookie;

    void init(boost::system::error_code & ec) {ec.clear();}
    void teardown(boost::system::error_code & ec) {ec.clear();}
 
   void Hello(ClientCookie & cc,ClientData & cd,
      basic_protocol::HelloRequest & hr, basic_protocol::HelloResponse & hres)
    { 
      if( m_backend->destination_type() != hr.expected_target() )
      {
        cd.error_code().assign(
          error_codes::RBL_BACKEND_CLIENT_TARGET_TYPE_MISMATCH,
          rpc_backend_error);
        cd.request_disconect();
        hres.set_error_type(basic_protocol::DESTINATION_EXPECTATION_MISMATCH);
        return;
      }
    
      if( m_backend->source_type() != hr.source_type())
      {
        cd.error_code().assign(
          error_codes::RBL_BACKEND_CLIENT_SOURCE_TYPE_MISMATCH,
          rpc_backend_error);
        hres.set_error_type(basic_protocol::SOURCE_EXPECTATION_MISMATCH);
        cd.request_disconect();
        return;
      }

      cd.name(hr.node_name());
      hres.set_error_type(basic_protocol::NO_HELLO_ERRORS);
      cd.establish_client();
    }
    void ListServices(  ClientCookie & cc ,ClientData & cd,
                        basic_protocol::ListServicesRequest & req, 
                        basic_protocol::ListServicesResponse & res)
    {
      BackEndBase::t_services services = m_backend->services();

      for(int i = 0; i < services.size(); ++i)
      {
        basic_protocol::ServiceEntry * s_e = res.add_services(); 
        const BackEndBase::t_services::entry_type * b_s_e = services.EntryAtordinal(i);
        s_e->set_service_ordinal( b_s_e->ordinal());
        s_e->set_service_name( b_s_e->name().c_str());
      } 
    }
  
    void backend(BackEndBase * backend)
      { m_backend = backend; }
  private:
     BackEndBase * m_backend;
  };

} }
#endif