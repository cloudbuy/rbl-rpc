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
  
  class BackEnd
  {
  public:
    typedef common::OidContainer<common::Oid, ServiceBase::shp> t_services;

    BackEnd    (  basic_protocol::SourceConnectionType       source_type,
                  basic_protocol::DestinationConnectionType backend_type);
    ~BackEnd();

    void pool_size(int pool_size_in) { m_pool_size = pool_size_in; }
    bool is_sealed() { return m_is_sealed; } 
    void seal() { m_is_sealed = true;}
   
    void start();
    void register_and_init_service(ServiceBase::shp service);
    void block_till_termination();
    bool shutdown();
  
 
    void connect(ClientData::ptr   client_data);
    void disconect(ClientData::ptr client_data);

    // invocation functions
    
    basic_protocol::SourceConnectionType source_type() const
      { return m_source_type;}
    basic_protocol::DestinationConnectionType destination_type() const
      { return m_backend_type; }
    const ClientServiceCookies & cookies() const 
      { return m_client_service_cookies; }

    template< typename Invoker>
    void invoke(Invoker & i);
    
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
    std::set<ClientData::ptr>                           m_connected_clients;    
 
    common::OidContainer<common::Oid, ServiceBase::shp> m_services;
    boost::uint16_t                                     m_service_count;
    bool                                                m_is_sealed;
    bool                                                m_has_shutdown;
 
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
   
 
     void backend ( BackEnd * backend );
  private:
     BackEnd * m_backend;
  };

  struct in_process_invoker
  {
    struct notification_object_
    { 
      typedef notification_object_ * ptr; 
      notification_object_()
      {
        reset();
      }
      void reset()
      {
        ready = false;
      }
      bool ready;
      boost::mutex mutex;
      boost::condition_variable cond;
    };

    in_process_invoker(BackEnd & b_in)
      : is_primary(true),
        client_data(new ClientData()),
        b(b_in),
        notification_object(new notification_object_()) 
    {
      b.connect(client_data);
    }

    in_process_invoker(const in_process_invoker & rhs)
      : is_primary(false),
        client_data(rhs.client_data),
        notification_object(rhs.notification_object),
        client_cookie(rhs.client_cookie),
        service(rhs.service),
        b(rhs.b)
    {
    }

    ~in_process_invoker()
    {
      if(is_primary)
      {

        b.disconect(client_data);
        delete client_data;
        delete notification_object;
      }
        
    }
  
    void reset()
    {
      notification_object->reset();
      client_data->request().Clear();
      client_data->response().Clear();
      client_data->error_code().clear();
      BOOST_ASSERT_MSG( client_data->is_rpc_active() == false, 
        "THE FLAG THAT REPRESENTS ACTIVE "
        "RPC SHOULD NOT BE SET WHEN RESETING AN OBJECT FOR RPC");
    }
    
    void operator() ()
    {

      service->dispatch(*client_cookie,*client_data);

      client_data->end_rpc();

      boost::lock_guard<boost::mutex> lock(notification_object->mutex);
      notification_object->ready=true;
      notification_object->cond.notify_one();

    }


    void after_post()
    {
      boost::unique_lock<boost::mutex> lock(notification_object->mutex);
      if(!notification_object->ready)
        notification_object->cond.wait(lock);
    }
    
    bool is_primary;
    ClientData::ptr client_data;
    notification_object_::ptr notification_object;
    ClientCookie * client_cookie;
    ServiceBase::ptr service;
    BackEnd & b;
  };

  template< typename Invoker>
  void BackEnd::invoke(Invoker & i)
  {
      i.client_data->start_rpc();

      basic_protocol::ClientRequest & request = i.client_data->request();
 
      ServiceBase::shp * service = 
        m_services[request.service_ordinal()];
      // check if service with ordinal exists
      if(service == NULL)
      {
        i.client_data->error_code().assign 
          ( error_codes::RBL_BACKEND_INVOKE_NO_SERVICE_WITH_ORDINAL_ERROR,
            rpc_backend_error);
        return; 
      }
    
      i.service = service->get();      

      // check if method ordinal is defined in the service
      if( ! i.service->contains_function_at_ordinal( request.request_ordinal() )) 
      {
        i.client_data->error_code().assign 
          ( error_codes::RBL_BACKEND_INVOKE_NO_REQUEST_WITH_ORDINAL_ERROR,
            rpc_backend_error);

        return;
      }
    
      { // lock_scope_lock
        boost::lock_guard<boost::recursive_mutex> lock(m_mutex);
        m_client_service_cookies.create_or_retrieve_cookie(
          request.service_ordinal(), i.client_data,&i.client_cookie);
      }
      
      // Check if subscribed, service 0 does not require an explicit subscribe 
      // event Subscription will be done implicetly
      if(request.service_ordinal() != 0)       
      {
        if( !i.client_cookie->is_subscribed())
        {
          i.client_data->error_code().assign(
            error_codes::RBL_BACKEND_INVOKE_CLIENT_NOT_SUBSCRIBED, 
            rpc_backend_error); 
          return; 
        }
      }
      
      if(request.service_ordinal() == 0 && request.request_ordinal() == 0)
      {
        if(i.client_data->is_client_established())
        {  
          i.client_data->error_code().assign(
            error_codes::RBL_BACKEND_ALLREADY_ESTABLISHED,
            rpc_backend_error);
          basic_protocol::HelloResponse hres;
          hres.set_error_type(basic_protocol::CLIENT_ALLREADY_ESTABLISHED);
          hres.SerializeToString( 
            i.client_data->response().mutable_response_string());

          i.client_data->request_disconect();
          return; 
        }
      }
      m_io_service.post(i);
      i.after_post();
    }

} }
#endif
