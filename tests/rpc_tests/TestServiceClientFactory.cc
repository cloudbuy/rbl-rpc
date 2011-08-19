#include <gtest/gtest.h>
#include <rpc/client/ClientServiceFactory.h>
#include <rpc/backend/BackEndBase.h>
#include <rpc/invoker/InProcessInvoker.h>
#include <rpc/backend/ClientServiceCookies.h>
#include <rpc/proto/TestService-server.rblrpc.h>

using namespace rubble::rpc;
using namespace rubble::rpc::test_proto;

class test_service_one_impl
{
public:
  typedef ClientCookieBase t_client_cookie;
  void init(boost::system::error_code & ec) { ec.clear(); }
  void subscribe(ClientCookie & client_cookie, ClientData & cd,
    std::string * in, std::string * out) 
  {
    BOOST_ASSERT(in->compare("hahaha")==0);
    out->assign("QQ");
  }
  void teardown(boost::system::error_code & ec) { ec.clear(); }
  void unsubscribe(ClientCookie & client_cookie, ClientData & cd) {}
  void dummy_rpc(ClientCookie &,ClientData &,Request & ,Response & ){}
private:
};

class FactorInProcessInvokerTest : public ::testing::Test
{
public:
  FactorInProcessInvokerTest()
    : b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL),
      s(new test_service_one<test_service_one_impl>()),
      invoker(b) {}

protected:
  void SetUp() 
  {
    b.register_and_init_service(s);
    b.pool_size(1);
    b.start();
    scf.reset(new ServiceClientFactory( "test service", invoker,
                                        basic_protocol::SOURCE_RELAY, 
                                        basic_protocol::TARGET_MARSHALL));
  }
  void TearDown()
  {
  } 

  BackEnd b;
  ServiceBase::shp s;
  InProcessInvoker  invoker;
  ServiceClientFactory::scptr scf;

};

TEST_F(FactorInProcessInvokerTest, compilation_test)
{
}

#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
  ::testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
#endif
