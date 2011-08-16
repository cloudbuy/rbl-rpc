#ifndef RBL_RPC_IN_PROCESS_INVOKER 
#define RBL_RPC_IN_PROCESS_INVOKER 
namespace rubble { namespace rpc {

  struct InProcessInvoker
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

    InProcessInvoker(BackEnd & b_in)
      : is_primary(true),
        client_data(new ClientData()),
        b(b_in),
        notification_object(new notification_object_()) 
    {
      b.connect(client_data);
    }
    
    // the primary flag below is there to mark the first invoker to be the p
    // primary object, when coppies are created they aren't primary.
    // A primary object releases resources on destroy.
    InProcessInvoker(const InProcessInvoker & rhs)
      : is_primary(false),
        client_data(rhs.client_data),
        notification_object(rhs.notification_object),
        client_cookie(rhs.client_cookie),
        service(rhs.service),
        b(rhs.b)
    {
    }

    ~InProcessInvoker()
    {
      if(is_primary)
      {
        BOOST_ASSERT_MSG( ( ! client_data->is_rpc_active()) , 
          "A PRIMARY INVOKER SHOULD NOT HAVE ACTIVE RPC WHEN IT IS DESTROYED") ;

        b.disconect(client_data);
        delete client_data;
        delete notification_object;
      }
        
    }
 
    bool is_useable()
    {
      return b.is_useable();       
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
      b.end_rpc(client_data);  

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

} }
#endif
