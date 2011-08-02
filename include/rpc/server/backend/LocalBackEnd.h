#ifndef RUBBLE_RPC_SYNCHJRONOUS_LOCAL_FRONTEND_H
#define RUBBLE_RPC_SYNCHJRONOUS_LOCAL_FRONTEND_H

#include <rpc/server/backend/BackEndBase.h>

namespace rubble { namespace rpc {

  struct local_invoker
  {
    struct notification_object_
    {  
      notification_object_()
      {
        reset();
      }
      void reset()
      {
        ready = false;
        ec.clear();
      }
      bool ready;
      boost::mutex mutex;
      boost::condition_variable cond;
      boost::system::error_code ec;
    };

    local_invoker(ClientData::shp & cd_in)
      : client_data(cd_in),
        notification_object(new notification_object_()) {}

    void reset()
    {
      notification_object->reset();
      client_data->request().Clear();
      client_data->response().Clear();
      client_data->error_code().clear();
      BOOST_ASSERT_MSG( client_data->is_rpc_active() == false, 
        "THE FLAG THAT REPRESENTS ACTIVE RPC SHOULD NOT BE SET WHEN RESETING AN OBJECT FOR RPC");
    }
    
    void operator() ()
    {
      
      service->dispatch(*client_cookie,*client_data.get());

      client_data->end_rpc();

      boost::lock_guard<boost::mutex> lock(notification_object->mutex);
      notification_object->ready=true;
      notification_object->cond.notify_one();
    }

    ClientData::shp client_data;
    boost::shared_ptr<notification_object_> notification_object;
    ClientCookie * client_cookie;
    ServiceBase::shp service;
  };

  class LocalBackEnd : public BackEndBase
  {
  public: 
    typedef local_invoker Invoker;    

    LocalBackEnd(  basic_protocol::SourceConnectionType       source_type
                  ,basic_protocol::DestinationConnectionType backend_type)
      : BackEndBase(source_type, backend_type) {}


    template< typename Invoker>
    void invoke(Invoker & i)
    {
      i.client_data->start_rpc();

      basic_protocol::ClientRequest & request = i.client_data->request();
 
      ServiceBase::shp * service = 
        m_services[request.service_ordinal()];
      // check if service with ordinal exists
      if(service == NULL)
      {
        i.notification_object->ec.assign 
          ( error_codes::RBL_BACKEND_INVOKE_NO_SERVICE_WITH_ORDINAL_ERROR,
            rpc_backend_error);
        return; 
      }
    
      i.service = *service;      

      // check if method ordinal is defined in the service
      if( ! (*service)->contains_function_at_ordinal( request.request_ordinal() )) 
      {
        i.notification_object->ec.assign 
          ( error_codes::RBL_BACKEND_INVOKE_NO_REQUEST_WITH_ORDINAL_ERROR,
            rpc_backend_error);

        return;
      }
    
      { // lock_scope_lock
        boost::shared_lock<boost::shared_mutex> lock(m_mutex);
        m_client_service_cookies.create_or_retrieve_cookie(
          request.service_ordinal(), i.client_data.get(),&i.client_cookie);
      }

      // Check if subscribed, service 0 does not a explicit subscribe event.
      // Subscription will be done implicetly
      if( request.service_ordinal() != 0)
      {
        if( !i.client_cookie->is_subscribed())
        {
          i.notification_object->ec.assign(
            error_codes::RBL_BACKEND_INVOKE_CLIENT_UNSUBSCRIBED, 
            rpc_backend_error);
        
          return; 
        }
      }
      m_io_service.post(i);
   
      boost::unique_lock<boost::mutex> lock(i.notification_object->mutex);
      if(!i.notification_object->ready)
        i.notification_object->cond.wait(lock);
    }

  }; 
} }

#endif 
