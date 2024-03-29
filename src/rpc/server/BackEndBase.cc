#include <rpc/backend/BackEndBase.h>

namespace rubble { namespace rpc {
// BackEnd ////////////////////////////////////////////////////////////////
  // BackEnd //////////////////////////////////////////////////////////////
  BackEnd::BackEnd( 
    basic_protocol::SourceConnectionType       source_type,
    basic_protocol::DestinationConnectionType backend_type) 
    : m_source_type(source_type),
      m_backend_type(backend_type),
      m_io_service(),
      m_work(new boost::asio::io_service::work(m_io_service)),
      m_services(),
      m_is_sealed(false),
      m_service_count(0),
      m_pool_size(0)
    {
      basic_protocol::basic_protocol<BasicProtocolImpl> * bp =
        new basic_protocol::basic_protocol<BasicProtocolImpl>();
      bp->impl().backend(this); 
      ServiceBase::shp sb_shp(bp);
      register_and_init_service(sb_shp);
    }
  //-------------------------------------------------------------------------//

  // ~BackEnd /////////////////////////////////////////////////////////////////
  BackEnd::~BackEnd()
  {
    shutdown();
  }
  //-------------------------------------------------------------------------//

  // start ////////////////////////////////////////////////////////////////////
  void BackEnd::start()
  {
    BOOST_ASSERT_MSG((m_pool_size > 0), "thread pool must have >=1 threads");

    if(m_is_sealed)
      throw BackEndException()
      <<  rbl_backend_error_code( boost::system::error_code( 
            error_codes::RBL_RPC_BACKEND_SEALED,rpc_backend_error));
    else
      m_is_sealed=true;
 
    BOOST_ASSERT_MSG(m_services.size() == m_services.occupied_size(), 
      "ordinal neels to be contigous");
    
    m_client_service_cookies.set_size(m_services.occupied_size());

    for(int i=0 ; i<m_services.occupied_size(); ++i)
    {
      m_client_service_cookies.activate_service_with_ordinal(i);  
    }
 
    for(int i=0;i < m_pool_size; ++i)
    {
      m_thread_group.create_thread(
        boost::bind( &boost::asio::io_service::run, &m_io_service) );
    }
  
    m_synchronised_state.begin_accepting_requests();  
  }
  //-------------------------------------------------------------------------//
  
  // is_useable ///////////////////////////////////////////////////////////////
  bool BackEnd::is_useable()
  {
    return (m_is_sealed);
  }
  //-------------------------------------------------------------------------//

  //  register_and_init_service ///////////////////////////////////////////////
  void BackEnd::register_and_init_service(ServiceBase::shp service)
  {
    if(!m_is_sealed)
    {
      common::OP_RESPONSE ret = m_services.SetEntry(
        common::Oid(service->name(),m_service_count),service);
      if(ret == common::OP_NO_ERROR)
      {
        service->init(m_ec);
        if(m_ec)
          throw BackEndException() 
            <<  rbl_backend_error_code(boost::system::error_code(
                  error_codes::RBL_RPC_BACKEND_INITIALIZATION_ERROR,
                  rpc_backend_error))
            <<  rbl_backend_service_name(service->name())
            <<  rbl_backend_service_error_code(
                  boost::system::error_code(m_ec));
        m_service_count++;
      }
      else throw BackEndException() 
        <<  rbl_backend_error_code( boost::system::error_code( 
            error_codes::RBL_RPC_BACKEND_IDENTIFIER_COLLISION,
            rpc_backend_error));
      }
      else throw BackEndException() 
        <<  rbl_backend_error_code( boost::system::error_code( 
              error_codes::RBL_RPC_BACKEND_SEALED, rpc_backend_error));
  }
  //-------------------------------------------------------------------------//
  
  // block_till_termination ///////////////////////////////////////////////////
  void BackEnd::block_till_termination()
  {
    m_thread_group.join_all();
  }
  //-------------------------------------------------------------------------//
  

  // shutdown /////////////////////////////////////////////////////////////////
  void BackEnd::shutdown(int step_seconds)
  {
    if( m_synchronised_state.check_and_initiate_shutdown() ) 
    {
      std::cout << "backend shutdown initiated" << std::endl;
      BackEndShutdownState  state = shutdown_step();
      
      while (state != BACKEND_SHUTDOWN_COMPLETE)
      {
        std::cout << "Not Complete, Waiting On: ";
  
        if( state == BACKEND_SHUTDOWN_WAITING_ON_ACTIVE_RPC_END)
          std::cout << "waiting on rpc end" << std::endl;
   
        if( state == BACKEND_SHUTDOWN_WAITING_ON_MANAGER_DISCONECTIONS)
          std::cout << manager_count() << " manager(s) and " 
                    << client_count()  << " client(s) to disconect."
                    << std::endl;

        boost::this_thread::sleep( 
          boost::posix_time::seconds(step_seconds) );
        
        state = shutdown_step();
      }
      std::cout << "backend shutdown complete" << std::endl;
    }
  }
  //-------------------------------------------------------------------------//

  // shutdown_step  ///////////////////////////////////////////////////////////
  BackEndShutdownState BackEnd::shutdown_step()
  {
    if( m_synchronised_state.shutdown_state() ==  BACKEND_SHUTDOWN_INITIATED)
    {
      m_synchronised_state.stop_accepting_requests();
    
      m_synchronised_state.shutdown_state(
        BACKEND_SHUTDOWN_WAITING_ON_ACTIVE_RPC_END);
    }
    
    if( m_synchronised_state.shutdown_state()  ==  
        BACKEND_SHUTDOWN_WAITING_ON_ACTIVE_RPC_END)
    {
      if( m_synchronised_state.rpc_count() == 0)
      {
        m_synchronised_state.shutdown_state(
          BACKEND_SHUTDOWN_WAITING_ON_MANAGER_DISCONECTIONS);
        m_synchronised_state.disconect_managers();
      }
      else 
        return BACKEND_SHUTDOWN_WAITING_ON_ACTIVE_RPC_END;
    }
   
    // TODO disconect client block.
    if( m_synchronised_state.shutdown_state() 
        == BACKEND_SHUTDOWN_WAITING_ON_MANAGER_DISCONECTIONS)
    {
      if( m_synchronised_state.manager_count() == 0) 
      { 
        BOOST_ASSERT_MSG(m_connected_clients.size() == 0,
          " Every connected client is managed by a connected manager. "
          " All clients should have been disconected if the managers have.");
      }
    }
    else
      return BACKEND_SHUTDOWN_WAITING_ON_MANAGER_DISCONECTIONS;  
    
    m_work.reset();
    m_thread_group.join_all();

    for(int i=0;i<m_services.occupied_size();++i)
    {
      ServiceBase::shp * sb_shp = m_services[i];
      if(sb_shp)
      {
        (*sb_shp)->teardown(m_ec);
        if(m_ec)
        {
          throw BackEndException()
            <<  rbl_backend_error_code( boost::system::error_code(
                  error_codes::RBL_BACKEND_TEARDOWN_ERROR, 
                  rpc_backend_error))
            <<  rbl_backend_service_name( (*sb_shp)->name())
            <<  rbl_backend_service_error_code( boost::system::error_code(
                  m_ec));
        }
        else
        {
          (*sb_shp).reset();
          BOOST_ASSERT_MSG((*sb_shp).use_count() == 0, 
            "shared pointer to service has stray references") ;
        }
      } 
    }    

    return BACKEND_SHUTDOWN_COMPLETE;
  }
  //-------------------------------------------------------------------------//
  
  // connect //////////////////////////////////////////////////////////////////
  void BackEnd::connect(ClientData::shptr client_data)
  {
    m_connected_clients.insert(client_data);
  }
  //-------------------------------------------------------------------------//

  // disconnect ////////////////////////////////////////////////////////////////
  void BackEnd::disconnect(ClientData::shptr client_data)
  {
    int m_sz = m_services.size();

    ClientCookie * client_cookie;
    ServiceBase::shp s;
   
    // if the backend is up and running, clean up any subscriptions for the 
    // client. 
    if(m_is_sealed && !m_io_service.stopped() )
    {
      for(int i = 1; i < m_sz; ++i)
      {
        m_client_service_cookies.create_or_retrieve_cookie(
          i, client_data.get(),&client_cookie);

        s = *(m_services[i]);

        if( client_cookie->is_subscribed())
        {
          s->unsubscribe(*client_cookie, *client_data);
        }
        client_cookie->destroy_cookie();
        m_client_service_cookies.delete_cookie(i,client_data.get());
      }
    }
    bool erased = (m_connected_clients.erase(client_data) == 1);
    BOOST_ASSERT_MSG(erased, 
      "the client was not present in the connected client set"); 
  }
  //-------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

// BasicProtocolImpl //////////////////////////////////////////////////////////
  // init /////////////////////////////////////////////////////////////////////
  void BasicProtocolImpl::init(boost::system::error_code & ec) 
  {
    ec.clear();
  }
  //-------------------------------------------------------------------------//

  // teardown /////////////////////////////////////////////////////////////////
  void BasicProtocolImpl::teardown(boost::system::error_code & ec) 
  {
    ec.clear();
  }
  //-------------------------------------------------------------------------//

  // hello ////////////////////////////////////////////////////////////////////
  void BasicProtocolImpl::hello(ClientCookie & cc,ClientData & cd,
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
  //-------------------------------------------------------------------------//
  
  // list_services ////////////////////////////////////////////////////////////
  void BasicProtocolImpl::list_services(  
    ClientCookie & cc, ClientData & cd,
    basic_protocol::ListServicesRequest & req, 
    basic_protocol::ListServicesResponse & res)
  
  {
    const BackEnd::t_services & services = m_backend->services();

    for(int i = 0; i < services.size(); ++i)
    {
      basic_protocol::ServiceEntry * s_e = res.add_services(); 
      
      const BackEnd::t_services::entry_type * b_s_e = 
        services.EntryAtordinal(i);
      
      s_e->set_service_ordinal( b_s_e->ordinal());
      s_e->set_service_name( b_s_e->name().c_str());
    } 
  }
  //-------------------------------------------------------------------------//

  // subscribe ////////////////////////////////////////////////////////////////
  void BasicProtocolImpl::subscribe(  ClientCookie & client_cookie, 
                                      ClientData & cd,std::string *,
                                      std::string *) 
  {

  }
  //-------------------------------------------------------------------------//

  // unsubscribe //////////////////////////////////////////////////////////////
  void BasicProtocolImpl::unsubscribe(  ClientCookie & client_cookie, 
                                        ClientData & cd) 
  {
  }
  //-------------------------------------------------------------------------//

  // rpc_subscribe_service ////////////////////////////////////////////////////
  void BasicProtocolImpl::rpc_subscribe_service( 
    ClientCookie & cc, ClientData & cd,
    basic_protocol::SubscribeServiceRequest & req,
    basic_protocol::SubscribeServiceResponse & res)
  { 
    BackEnd::t_services & services = m_backend->services();
    
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
    
    cookies.create_or_retrieve_cookie(req.service_ordinal(), &cd, &cookie);
    
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

    if(!cd.error_code())
    {
      cookie->subscribe();
      res.set_error(basic_protocol::NO_SUBSCRIBE_SERVICE_ERROR);
    }
  }
  //-------------------------------------------------------------------------//

  // rpc_unsubscribe_service //////////////////////////////////////////////////
  void BasicProtocolImpl::rpc_unsubscribe_service ( 
    ClientCookie & cc,ClientData & cd,
    basic_protocol::UnsubscribeServiceRequest & req,
    basic_protocol::UnsubscribeServiceResponse & res )
  {

    BackEnd::t_services & services = m_backend->services();

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
    cookies.create_or_retrieve_cookie( req.service_ordinal(), &cd, &cookie);
    
    if(! cookie->is_subscribed())
    {
      cd.error_code().assign(
        error_codes::RBL_BACKEND_NOT_SUBSCRIBED,
        rpc_backend_error);
      res.set_error(basic_protocol::NO_SUBSCRIPTION_FOR_SERVICE);
      return;
    }

    ServiceBase::shp & s = * services[req.service_ordinal()];
    s->unsubscribe(*cookie,cd);

    cookie->destroy_cookie();
    cookies.delete_cookie(req.service_ordinal(),&cd);
    
    res.set_error(basic_protocol::NO_SUBSCRIBE_SERVICE_ERROR);
  }
  //-------------------------------------------------------------------------//
  
  // list_methods /////////////////////////////////////////////////////////////
  void BasicProtocolImpl::list_methods (
      ClientCookie & cc,   ClientData & cd ,
      basic_protocol::ListMethodsRequest & req ,
      basic_protocol::ListMethodsResponse & res )
  {
    BackEnd::t_services & services = m_backend->services();
    
    if( !( req.service_ordinal() < services.size()))
    {
      cd.error_code().assign( 
        error_codes::RBL_BACKEND_LIST_METHOD_NO_SERVICE_WITH_ORDINAL,
        rpc_backend_error);
      res.set_error(basic_protocol::NO_SERVICE_WITH_ORDINAL);
      return;
    }
    
    ServiceBase::shp * service = services[req.service_ordinal()];
    (*service)->produce_method_list(res);
    res.set_error(basic_protocol::NO_LIST_METHOD_ERROR);
  }

  //-------------------------------------------------------------------------//
    
  // backend ////////////////////////////////////////////////////////////////// 
  void BasicProtocolImpl::backend(BackEnd * backend)
  { 
    m_backend = backend; 
  }
  //-------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
} }
