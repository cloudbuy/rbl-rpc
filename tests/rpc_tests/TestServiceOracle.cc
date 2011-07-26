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
    void init(boost::system::error_code & ec) { }
    void teardown(boost::system::error_code & ec) {}
    bool dummy_rpc(ClientCookie &,ClientData &,Request & ,Response & ){}
  private:
  };


TEST(service_tests,all)
{
  BackEnd b;
  ServiceBase_shp s(new test_service_one<test_service_one_no_fail>());
  b.register_and_init_service(s);
 
  //ServiceBase_shp s1(new BasicProtocol<BasicProtocol_cons_fail>());
  //so.register_and_init_service(s1);
}

#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
#endif
