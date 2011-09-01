#ifndef RBL_RPC_IN_PROCESS_INVOKER 
#define RBL_RPC_IN_PROCESS_INVOKER 
#include <rpc/invoker/InvokerBase.h>

namespace rubble { namespace rpc {
  struct InProcessInvoker : public InvokerBase
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
      : b(b_in),
        notification_object(new notification_object_()) 
    {
      b.connect(m_client_data);
    }
    
    ~InProcessInvoker()
    {
      if( m_client_data.unique() )
      {
        b.disconect(m_client_data); 
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
      m_client_data->request().Clear();
      m_client_data->response().Clear();
      m_client_data->error_code().clear();
      BOOST_ASSERT_MSG( m_client_data->is_rpc_active() == false, 
        "THE FLAG THAT REPRESENTS ACTIVE "
        "RPC SHOULD NOT BE SET WHEN RESETING AN OBJECT FOR RPC");
    }
   
    void invoke()
    {
      b.invoke(*this);
    }
 
    void operator() ()
    {
      service->dispatch(*client_cookie,*m_client_data);
      b.end_rpc(m_client_data.get());  

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
    
    notification_object_::ptr notification_object;
    ClientCookie * client_cookie;
    ServiceBase::ptr service;
    BackEnd & b;
  };

} }
#endif
