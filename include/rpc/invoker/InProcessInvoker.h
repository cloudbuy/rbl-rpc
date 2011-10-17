#ifndef RBL_RPC_IN_PROCESS_INVOKER 
#define RBL_RPC_IN_PROCESS_INVOKER 
#include <rpc/invoker/InvokerBase.h>

namespace rubble { namespace rpc {
  struct NotificationObject
  { 
    typedef NotificationObject *                  ptr;
    typedef boost::shared_ptr<NotificationObject> shptr;

    NotificationObject()
    {
      reset();
    }
    void reset()
    {
      ready = false;
    }
    
    void notify_one()
    {
      boost::lock_guard<boost::mutex> lock(mutex);
      ready=true;
      cond.notify_one();
    }
    
    void notify_all()
    {
      boost::lock_guard<boost::mutex> lock(mutex);
      ready=true;
      cond.notify_all();
    }

    void wait_for_notification()
    {
       boost::unique_lock<boost::mutex> lock(mutex);
      
      if(!ready)
        cond.wait(lock);
    }
    
    bool ready;
    boost::mutex mutex;
    boost::condition_variable cond;
  };

  struct InProcessInvoker : InvokerBase
  {
    typedef boost::shared_ptr<InProcessInvoker> shptr;


    InProcessInvoker(BackEnd & b_in)
      : InvokerBase(),
        m_backend(b_in),
        notification_object(),
        m_connected(false)
    {
      m_sig_connection =  m_backend.register_invoker_manager(*this);
    }
    
    bool is_connected()
    {
      return m_connected;
    }
    
    ~InProcessInvoker()
    {
      if( m_client_data.unique() )
      {
        if(m_connected) 
          disconect_from_backend();
      }
    }
 
    // called by the register_invoker_manager frunction 
    void connect_to_backend()
    {
      BOOST_ASSERT(m_connected == false);
      m_backend.connect(m_client_data);
      m_connected = true;
    }
  
    void disconect_from_backend()
    {
      boost::lock_guard<boost::mutex> lock(m_mutex);

      if(m_connected)
      {
        m_backend.disconnect(m_client_data); 
        m_connected = false;
      }
     
      m_sig_connection->disconnect(); 
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
   
    bool invoke()
    {
      boost::lock_guard<boost::mutex> lock(m_mutex);
      if( m_connected == false )
        throw InvokerException(); 
     
      return m_backend.invoke(*this);
    }
 
    void operator() ()
    {
      service->dispatch(*client_cookie,*m_client_data);
      m_backend.end_rpc(m_client_data.get());  

      notification_object.notify_one();
    }

    void after_post()
    {
      notification_object.wait_for_notification();
    }
    
    NotificationObject                    notification_object;
    BackEnd &                             m_backend;
    bool                                  m_connected;
    SynchronisedSignalConnection::aptr    m_sig_connection;
    boost::mutex                          m_mutex;
  };

} }
#endif
