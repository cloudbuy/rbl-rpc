#include <gtest/gtest.h>
#include <rpc/server/ClientServiceCookies.h>
#include <rpc/server/backend/backend.h>
#include <rpc/proto/TestService-server.rblrpc.h>

using namespace rubble::rpc;
using namespace rubble::rpc::test_proto;

  class test_service_one_no_fail
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.clear(); }
    void teardown(boost::system::error_code & ec) {}
    bool dummy_rpc(ClientCookie &,ClientData &,Request & ,Response & ){}
  private:
  };

  class test_service_one_fail
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.assign(1,rpc_backend_error); }
    void teardown(boost::system::error_code & ec) {}
    bool dummy_rpc(ClientCookie &,ClientData &,Request & ,Response & ){}
  private:
  };

  class test_service_one_dest_no_fail
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.clear(); }
    void teardown(boost::system::error_code & ec) {ec.clear();}
    bool dummy_rpc(ClientCookie &,ClientData &,Request & ,Response & ){}
  private:
  };

  class test_service_one_dest_fail
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.clear(); }
    void teardown(boost::system::error_code & ec) { ec.assign(1,rpc_backend_error); }
    bool dummy_rpc(ClientCookie &,ClientData &,Request & ,Response & ){}
  private:
  };

  class test_service_one_impl
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.clear(); }
    void teardown(boost::system::error_code & ec) { ec.clear(); }
    bool dummy_rpc(ClientCookie &,ClientData &,Request & ,Response & ){}
  private:
  };

TEST(backed_tests,duplicate_identifier_fail)
{
  BackEnd b;
  ServiceBase_shp s(new test_service_one<test_service_one_no_fail>());
  b.register_and_init_service(s);

  ASSERT_THROW( b.register_and_init_service(s),BackEndException);
}

TEST(backed_tests,service_innit_fail)
{
  BackEnd b;
  ServiceBase_shp s(new test_service_one<test_service_one_fail>());
  
  ASSERT_THROW( b.register_and_init_service(s),BackEndException);

}

TEST(backed_tests,teardown_no_fail)
{
  BackEnd b;
  ServiceBase_shp s(new test_service_one<test_service_one_dest_no_fail>());
  b.register_and_init_service(s);
  
  b.pool_size(1);
  b.start();
  ASSERT_NO_THROW(b.shutdown());
}

TEST(backed_tests,teardown_fail)
{
  BackEnd b;
  ServiceBase_shp s(new test_service_one<test_service_one_dest_fail>());
  b.register_and_init_service(s);
  
  b.pool_size(1);
  b.start();
  ASSERT_THROW(b.shutdown(),BackEndException);
}

  struct synchronous_rpc_invoker
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

    synchronous_rpc_invoker(ClientData::shp & cd_in)
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
    ServiceBase_shp service;
  };


TEST(backed_tests, connect_hello_list)
{
  BackEnd b;
  ServiceBase_shp s(new test_service_one<test_service_one_impl>());
  b.register_and_init_service(s);
  
  b.pool_size(1);
  b.start();

  ClientData::shp cd(new ClientData());

  b.connect(cd);

  basic_protocol::HelloRequest hello;
  hello.set_source_type( basic_protocol::SOURCE_RELAY);
  hello.set_expected_target( basic_protocol::TARGET_MARSHALL); 
  hello.set_node_name("test_client");

  cd->request().Clear(); 
  cd->request().set_service_ordinal(0);
  cd->request().set_request_ordinal(0);
  hello.SerializeToString(cd->request().mutable_request_string());
  
  synchronous_rpc_invoker invoker(cd);
  b.synchronous_invoke(invoker); 

  ASSERT_FALSE(cd->is_rpc_active()); 

  b.shutdown();
}

#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
#endif
