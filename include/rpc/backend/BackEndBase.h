#ifndef RBL_RPC_BACKEND
#define RBL_RPC_BACKEND
#include <rpc/common/rpc_errors.h>
#include <rpc/backend/ClientData.h>
#include <rpc/backend/ClientServiceCookies.h>
#include <rpc/backend/ServiceBase.h>
#include <rpc/proto/BasicProtocol-server.rblrpc.h>
#include <rpc/client/ClientBase.h>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/signal.hpp>
#include <set>
#include <iostream>

#include <functional>

#define BASIC_PROTOCOL_HELLO_ORDINAL                0
#define BASIC_PROTOCOL_LIST_SERVICES_ORDINAL        1
#define BASIC_PROTOCOL_RPC_SUBSCRIBE_ORDINAL        2
#define BASIC_PROTOCOL_RPC_UBSUBSCRIBE_ORDINAL      3
#define BASIC_PROTOCOL_LIST_METHODS_ORDINAL         4

namespace rubble { namespace rpc {
  enum BackEndShutdownState
  {
    BACKEND_ACTIVE = 0,
    BACKEND_SHUTDOWN_INITIATED,
    BACKEND_SHUTDOWN_WAITING_ON_ACTIVE_RPC_END,
    BACKEND_SHUTDOWN_WAITING_ON_MANAGER_DISCONECTIONS,
    BACKEND_SHUTDOWN_COMPLETE
  };

// class SynchronisedSignalConnection //////////////////////////////////////////
  class SynchronisedSignalConnection
  {
  public:
    typedef std::auto_ptr<SynchronisedSignalConnection> aptr;

    SynchronisedSignalConnection( boost::recursive_mutex & mutex ,  
                                  boost::signals::connection & con)
      :  m_connection(con),m_mutex(mutex) {}
    
    bool is_connected()
    {
      boost::lock_guard<boost::recursive_mutex> lock(m_mutex);
      return m_connection.connected();
    }

    bool disconnect()
    { 
      boost::lock_guard<boost::recursive_mutex> lock(m_mutex);
      
      if( m_connection.connected() )
      {
        m_connection.disconnect();
        return true;
      }
      else
        return false;
    }
  private :
    boost::signals::connection  m_connection;
    boost::recursive_mutex &    m_mutex;
  };
//----------------------------------------------------------------------------//

// class SynchronisedBackEndState //////////////////////////////////////////////
  class SynchronisedBackEndState
  {
  public:
    SynchronisedBackEndState()
      : m_active_rpc_count(0),
        m_accepting_requests(false),
        m_shutdown_state(BACKEND_ACTIVE)
    { 
    }

    bool start_a_request()
    {
      boost::lock_guard<boost::recursive_mutex> lock (m_mutex);
  
      if(m_accepting_requests)
      {
        m_active_rpc_count++;
        return true;
      }
      else
        return false;
    }
  
    boost::int32_t rpc_count()
    {
      boost::lock_guard<boost::recursive_mutex> lock (m_mutex);
      return m_active_rpc_count;
    }
  
    void end_a_request()
    {
      boost::lock_guard<boost::recursive_mutex> lock (m_mutex);
      m_active_rpc_count--;
    }
    
    void begin_accepting_requests()
    {
      boost::lock_guard<boost::recursive_mutex> lock (m_mutex);
      m_accepting_requests = true;
    }

    void stop_accepting_requests()
    {
      boost::lock_guard<boost::recursive_mutex> lock (m_mutex);
      m_accepting_requests = false;
    }
    
    bool is_accepting_requests()
    {
      boost::lock_guard<boost::recursive_mutex> lock (m_mutex);
      return m_accepting_requests;
    }

    template<typename Manager>
    SynchronisedSignalConnection::aptr  register_invoker_manager(Manager & m)
    {
      boost::lock_guard<boost::recursive_mutex> lock (m_mutex);
      m.connect_to_backend();
      
      boost::signals::connection con = f_disc_invoker_sig.connect(
             boost::bind( &Manager::disconnect_from_backend, &m));

      return SynchronisedSignalConnection::aptr( 
        new SynchronisedSignalConnection( 
          m_mutex, con
        )
      );
    }

    std::size_t manager_count()
    {
      boost::lock_guard<boost::recursive_mutex> lock (m_mutex);
      return f_disc_invoker_sig.num_slots();
    }

    void disconect_managers()
    {
      boost::lock_guard<boost::recursive_mutex> lock (m_mutex);
      
      f_disc_invoker_sig();
    }

    bool check_and_initiate_shutdown()
    {
      boost::lock_guard<boost::recursive_mutex> lock (m_mutex);
      if( m_shutdown_state ==  BACKEND_ACTIVE)
      {
        m_shutdown_state = BACKEND_SHUTDOWN_INITIATED;
        return true;
      }
      else
        return false;
    }
    void shutdown_state(BackEndShutdownState state)
    {
      m_shutdown_state = state;
    }
  
    BackEndShutdownState shutdown_state()
    {
      return m_shutdown_state;
    }
  private:
    bool                    m_accepting_requests;
    boost::int32_t          m_active_rpc_count;
    boost::recursive_mutex  m_mutex;
    boost::signal<void() >  f_disc_invoker_sig;
    BackEndShutdownState    m_shutdown_state;
  };
//----------------------------------------------------------------------------//

// class BackEnd /////////////////////////////////////////////////////////////// 
  class BackEnd : boost::noncopyable
  {
  public:
    typedef boost::shared_ptr<BackEnd> shptr;
    typedef common::OidContainer<common::Oid, ServiceBase::shp> t_services;

    BackEnd    (  basic_protocol::SourceConnectionType       source_type,
                  basic_protocol::DestinationConnectionType     backend_type);
    ~BackEnd();

    void pool_size(int pool_size_in) { m_pool_size = pool_size_in; }
    bool is_sealed() { return m_is_sealed; }
   
    void pause_requests() { m_synchronised_state.stop_accepting_requests(); }
    void resume_requests() { m_synchronised_state.begin_accepting_requests(); } 

    void start();
    bool is_useable();
    boost::int32_t rpc_count();
    void end_rpc(ClientData::ptr client_data); 
    void register_and_init_service(ServiceBase::shp service);
    void block_till_termination();

    void shutdown(int step_seconds = 5);
    BackEndShutdownState shutdown_step();
    
 
    void connect(ClientData::shptr   client_data);
    void disconnect(ClientData::shptr client_data);

    std::size_t manager_count();
    std::size_t client_count();
 
    template<typename Manager>
    SynchronisedSignalConnection::aptr register_invoker_manager(Manager & m)
    {
      return m_synchronised_state.register_invoker_manager(m);
    }


    basic_protocol::SourceConnectionType source_type() const
      { return m_source_type;}
    basic_protocol::DestinationConnectionType destination_type() const
      { return m_backend_type; }
    const ClientServiceCookies & cookies() const 
      { return m_client_service_cookies; }

    template< typename Invoker>
    bool invoke(Invoker & i);
  protected:
    friend class BasicProtocolImpl;

    t_services & services() { return m_services;}
    ClientServiceCookies & cookies() { return m_client_service_cookies; }

    basic_protocol::SourceConnectionType                m_source_type;
    basic_protocol::DestinationConnectionType           m_backend_type;

    int                                                 m_pool_size;
    boost::thread_group                                 m_thread_group;
    boost::system::error_code                           m_ec;

    ClientServiceCookies                                m_client_service_cookies;
    ConnectedClientsSet                                 m_connected_clients;    
    SynchronisedBackEndState                            m_synchronised_state;

 
    common::OidContainer<common::Oid, ServiceBase::shp> m_services;
    boost::uint16_t                                     m_service_count;
    bool                                                m_is_sealed;

    boost::asio::io_service                             m_io_service;

    boost::scoped_ptr<boost::asio::io_service::work>    m_work;
    
  };
//----------------------------------------------------------------------------//

// class BasicProtocolImpl /////////////////////////////////////////////////////
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
   
 
     void backend ( BackEnd * backend );
  private:
     BackEnd * m_backend;
  };
//----------------------------------------------------------------------------//

  inline boost::int32_t BackEnd::rpc_count()
  {
    return m_synchronised_state.rpc_count();
  }

  inline std::size_t BackEnd::manager_count()
  {
    return m_synchronised_state.manager_count(); 
  }

  inline std::size_t BackEnd::client_count()
  {
    return m_connected_clients.size();
  }


  inline void BackEnd::end_rpc(ClientData::ptr client_data)
  {
    m_synchronised_state.end_a_request();
    client_data->end_rpc();
  }

  
  template< typename Invoker>
  bool BackEnd::invoke(Invoker & i)
  {
    ClientData::shptr client_data = i.client_data();

    // reset the error state
    client_data->response().set_error(basic_protocol::REQUEST_SUCCESS);
    client_data->error_code().clear();

    if( m_synchronised_state.start_a_request() )
    {
      client_data->start_rpc();
    }
    else 
    {
      client_data->error_code().assign(                                         
        error_codes::RBL_BACKEND_NOT_ACCEPTING_REQUESTS, rpc_backend_error);    
      client_data->response().set_error(
        basic_protocol::REQUEST_BACKEND_NOT_ACCEPTING_REQUESTS);
      return true;                                                                   
    } 

    basic_protocol::ClientRequest & request = client_data->request();

    ServiceBase::shp * service = 
      m_services[request.service_ordinal()];

    // check if service with ordinal exists
    if(service == NULL)
    {
      client_data->error_code().assign 
        ( error_codes::RBL_BACKEND_INVOKE_NO_SERVICE_WITH_ORDINAL_ERROR,
          rpc_backend_error);
      client_data->response().set_error(basic_protocol::REQUEST_NO_SERVICE_WITH_ORDINAL);
      end_rpc(client_data.get());
      return true;
    }
 
    i.service = service->get();      

    // check if method ordinal is defined in the service
    if( ! i.service->contains_function_at_ordinal( request.request_ordinal() )) 
    {
      client_data->error_code().assign 
        ( error_codes::RBL_BACKEND_INVOKE_NO_REQUEST_WITH_ORDINAL_ERROR,
          rpc_backend_error);
      client_data->response().set_error(basic_protocol::REQUEST_NO_FUNCTION_WITH_ORDINAL);
      end_rpc(client_data.get());
      return true;
    }

    m_client_service_cookies.create_or_retrieve_cookie(
      request.service_ordinal(), i.client_data().get(),&i.client_cookie);

    // Check if subscribed, service 0 does not require an explicit subscribe 
    // event Subscription will be done implicetly
    if(request.service_ordinal() != 0)       
    {
      if( ! i.client_cookie->is_subscribed())
      {
        client_data->error_code().assign(
          error_codes::RBL_BACKEND_INVOKE_CLIENT_NOT_SUBSCRIBED, 
          rpc_backend_error); 
        client_data->response().set_error(basic_protocol::REQUEST_NOT_SUBSCRIBED);
        end_rpc(client_data.get());
        return true;
      }
    }
    
    if(request.service_ordinal() == 0 && request.request_ordinal() == 0)
    {
      if(client_data->is_client_established())
      {  
        client_data->error_code().assign(
          error_codes::RBL_BACKEND_ALLREADY_ESTABLISHED,
          rpc_backend_error);
        basic_protocol::HelloResponse hres;
        hres.set_error_type(basic_protocol::CLIENT_ALLREADY_ESTABLISHED);
        hres.SerializeToString( 
          client_data->response().mutable_response_string());

        client_data->request_disconect();

        end_rpc(client_data.get());
        return true;
      }
    } 
    else if( ! client_data->is_client_established() )// client must be established
    {
      client_data->error_code().assign(
        error_codes::RBL_BACKEND_NOT_ESTABLISHED,
        rpc_backend_error);
      client_data->response().set_error(basic_protocol::REQUEST_CLIENT_NOT_ESTABLISHED);
      end_rpc(client_data.get());
      return true; 
    }
    
//    std::cout << (*service)->name() << "::" << request.request_ordinal() << std::endl; 
    m_io_service.post(boost::bind(&Invoker::operator(),boost::ref(i)));
    i.after_post();
    return false;
  }

} }
#endif
