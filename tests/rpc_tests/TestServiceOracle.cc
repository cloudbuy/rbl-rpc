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
    bool dummy_rpc(ClientCookie *,ClientData &,Request & ,Response & ){}
  private:
  };

  class test_service_one_fail
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.assign(1,rpc_backend_error); }
    void teardown(boost::system::error_code & ec) {}
    bool dummy_rpc(ClientCookie *,ClientData &,Request & ,Response & ){}
  private:
  };

  class test_service_one_dest_no_fail
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.clear(); }
    void teardown(boost::system::error_code & ec) {ec.clear();}
    bool dummy_rpc(ClientCookie *,ClientData &,Request & ,Response & ){}
  private:
  };

  class test_service_one_dest_fail
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.clear(); }
    void teardown(boost::system::error_code & ec) { ec.assign(1,rpc_backend_error); }
    bool dummy_rpc(ClientCookie *,ClientData &,Request & ,Response & ){}
  private:
  };

  class test_service_one_impl
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.clear(); }
    void teardown(boost::system::error_code & ec) { ec.clear(); }
    bool dummy_rpc(ClientCookie *,ClientData &,Request & ,Response & ){}
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

  struct simple_rpc_invoker
  {
    void operator()(  boost::system::error_code ec,
                      rubble::common::OidContainer<common::Oid, ServiceBase_shp> * services, 
                      ClientData_shp cd_shp)
    {
      ready=false;

      boost::unique_lock<boost::mutex> lock(mutex);
      ready = true;
      cond.notify_one();
    }

    bool ready;
    boost::mutex mutex;
    boost::condition_variable cond;
  };


TEST(backed_tests, connect_hello_list)
{
  BackEnd b;
  ServiceBase_shp s(new test_service_one<test_service_one_impl>());
  b.register_and_init_service(s);
  
  b.pool_size(1);
  b.start();

  ClientData_shp cd(new ClientData());

  b.connect(cd);

  basic_protocol::HelloRequest hello;
  hello.set_source_type( basic_protocol::SOURCE_RELAY);
  hello.set_expected_target( basic_protocol::TARGET_MARSHALL); 
  hello.set_node_name("test_client");

  cd->request().Clear(); 
  cd->request().set_service_ordinal(0);
  cd->request().set_request_ordinal(0);
  hello.SerializeToString(cd->request().mutable_request_string());

  

  b.shutdown();
}

#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
#endif
