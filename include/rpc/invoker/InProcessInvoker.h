#ifndef RBL_RPC_IN_PROCESS_INVOKER 
#define RBL_RPC_IN_PROCESS_INVOKER 
#include <rpc/invoker/InvokerBase.h>

namespace rubble { namespace rpc {
  struct InProcessInvoker : InvokerBase
  {
    typedef boost::shared_ptr<InProcessInvoker> shptr;

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
      : InvokerBase(),
        m_backend(b_in),
        notification_object(),
        m_connected(false)
    {
      m_backend.register_invoker_manager(*this);
    }
    
    ~InProcessInvoker()
    {
      if( m_client_data.unique() )
      {
        
        disconect_from_backend();
      }
    }
  
    void connect_to_backend()
    {
      BOOST_ASSERT(m_connected == false);
      m_backend.connect(m_client_data);
      m_connected = true;
    }
  
    void disconect_from_backend()
    {
      if(m_connected)
      {
        m_backend.disconect(m_client_data); 
        m_connected = false;
      }
    }

    bool is_useable()
    {
      return m_backend.is_useable();       
    }
 
    void reset()
    {
      notification_object.reset();
      m_client_data->request().Clear();
      m_client_data->response().Clear();
      m_client_data->error_code().clear();
      BOOST_ASSERT_MSG( m_client_data->is_rpc_active() == false, 
        "THE FLAG THAT REPRESENTS ACTIVE "
        "RPC SHOULD NOT BE SET WHEN RESETING AN OBJECT FOR RPC");
    }
   
    void invoke()
    {
      m_backend.invoke(*this);
    }
 
    void operator() ()
    {
      service->dispatch(*client_cookie,*m_client_data);
      m_backend.end_rpc(m_client_data.get());  

      boost::lock_guard<boost::mutex> lock(notification_object.mutex);
      notification_object.ready=true;
      notification_object.cond.notify_one();
    }

    void after_post()
    {
      boost::unique_lock<boost::mutex> lock(notification_object.mutex);
      if(!notification_object.ready)
        notification_object.cond.wait(lock);
    }
    
    notification_object_  notification_object;
    BackEnd &             m_backend;
    bool                  m_connected;
  };

} }
#endif
