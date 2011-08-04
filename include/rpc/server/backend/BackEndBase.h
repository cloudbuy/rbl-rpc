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
  
 
    void connect(ClientData::shp & client_data);
    void disconect(ClientData::shp & client_data);
    
    basic_protocol::SourceConnectionType source_type() const
      { return m_source_type;}
    basic_protocol::DestinationConnectionType destination_type() const
      { return m_backend_type; }
  protected:
    friend class BasicProtocolImpl;

    const t_services & services() const { return m_services;}
    ClientServiceCookies & cookies() { return m_client_service_cookies; }
    boost::shared_mutex & mutex() { return m_mutex; }

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
 
    void hello(ClientCookie & cc,ClientData & cd,
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

    void list_services(  ClientCookie & cc ,ClientData & cd,
                        basic_protocol::ListServicesRequest & req, 
                        basic_protocol::ListServicesResponse & res)
    {
      const BackEndBase::t_services & services = m_backend->services();

      for(int i = 0; i < services.size(); ++i)
      {
        basic_protocol::ServiceEntry * s_e = res.add_services(); 
        const BackEndBase::t_services::entry_type * b_s_e = services.EntryAtordinal(i);
        s_e->set_service_ordinal( b_s_e->ordinal());
        s_e->set_service_name( b_s_e->name().c_str());
      } 
    }
    void subscribe(ClientCookie & client_cookie, ClientData & cd,
      std::string *,std::string *) {}
    void unsubscribe(ClientCookie & client_cookie, ClientData & cd) {}

    void rpc_subscribe_service( ClientCookie & cc,ClientData & cd,
                                basic_protocol::SubscribeServiceRequest & req,
                                basic_protocol::SubscribeServiceResponse & res)
    { 
      BackEndBase::t_services services = m_backend->services();
      
      if( !( req.service_ordinal() < services.size()))
      {
        cd.error_code().assign( 
          error_codes::RBL_BACKEND_SUBSCRIBE_NO_SERVICE_WITH_ORDINAL,
          rpc_backend_error);
        res.set_error(basic_protocol::SERVICE_ORDINAL_NOT_IN_USE);
        return;
      }

      ClientServiceCookies & cookies = m_backend->cookies(); 
      ClientCookie * cookie;
      { 
        boost::shared_mutex & mutex = m_backend->mutex();
        boost::unique_lock<boost::shared_mutex> lock(mutex);
        cookies.create_or_retrieve_cookie(
          req.service_ordinal(), &cd, &cookie);
      }
      
      //subcription should only occur once
      if(cookie->is_subscribed())
      {
        cd.error_code().assign(
          error_codes::RBL_BACKEND_ALLREADY_SUBSCRIBED,
          rpc_backend_error);
        res.set_error(basic_protocol::SERVICE_ALLREADY_SUBSCRIBED);
        return;
      } 

      ServiceBase::shp * s = services[req.service_ordinal()];

      BOOST_ASSERT_MSG(s != NULL, 
      "THERE SHOULD BE A SERVICE IN THE CONTEXT OF A " 
      "CLIENT-SERVICE-SUBSCRIPTION");
      
      std::string * req_s = NULL;  
      if(req.has_subscribe_request_string())
        req_s = req.mutable_subscribe_request_string();
 
      std::string * res_s = 
        res.mutable_subscribe_result_string();
      
      (*s)->subscribe (*cookie,cd,req_s,res_s);
      res.set_error(basic_protocol::NO_SUBSCRIBE_SERVICE_ERROR);
    }

    void backend(BackEndBase * backend)
      { m_backend = backend; }
  private:
     BackEndBase * m_backend;
  };

} }
#endif
