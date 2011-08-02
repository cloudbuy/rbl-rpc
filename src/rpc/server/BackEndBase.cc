#include <rpc/server/backend/BackEndBase.h>

namespace rubble { namespace rpc {
  BackEndBase::BackEndBase( 
    basic_protocol::SourceConnectionType       source_type,
    basic_protocol::DestinationConnectionType backend_type) 
    : m_source_type(source_type),
      m_backend_type(backend_type),
      m_io_service(),
      m_work(m_io_service),
      m_services(),
      m_is_sealed(false),
      m_service_count(0) 

    {
      basic_protocol::basic_protocol<BasicProtocolImpl> * bp =
        new basic_protocol::basic_protocol<BasicProtocolImpl>();
      bp->impl().backend(this); 
      ServiceBase::shp sb_shp(bp);
      
      register_and_init_service(sb_shp);
    }

  void BackEndBase::start()
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
  }
  
  void BackEndBase::register_and_init_service(ServiceBase::shp service)
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
  
  void BackEndBase::block_till_termination()
  {
    m_thread_group.join_all();
  }

  bool BackEndBase::shutdown()
  {
    BOOST_ASSERT_MSG(m_is_sealed, 
      "shutdown should not be run on an unsealed backend");
    m_io_service.stop();
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

    return m_io_service.stopped();
  }

  void BackEndBase::connect(ClientData::shp & client_data)
  {
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    m_connected_clients.insert(client_data);
  }
  void BackEndBase::disconect(ClientData::shp & client_data)
  {
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    m_connected_clients.erase(client_data);
  }

} }
