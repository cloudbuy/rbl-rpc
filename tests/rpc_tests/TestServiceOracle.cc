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

#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
#endif
