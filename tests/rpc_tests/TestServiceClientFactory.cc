#include <gtest/gtest.h>
#include <rpc/client/ClientServiceFactory.h>
#include <rpc/backend/BackEndBase.h>
#include <rpc/invoker/InProcessInvoker.h>
#include <rpc/backend/ClientServiceCookies.h>
#include <rpc/proto/TestService-server.rblrpc.h>
#include <rpc/proto/TestService-client.rblrpc.h>

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
    test_proto::subscribe_data_in in_;
    test_proto::subscribe_data_out out_;

    in_.ParseFromString(*in);
  
    if( in_.str().compare("okie") == 0)
    {
      out_.set_str("dokie");
    }
    else
      out_.set_str("pokie");
    
    out_.SerializeToString(out);
  }
  void teardown(boost::system::error_code & ec) { ec.clear(); }
  void unsubscribe(ClientCookie & client_cookie, ClientData & cd) {}
  void dummy_rpc(ClientCookie &,ClientData &,Request & req,Response & res)
  {
    if( req.req().compare("okie") == 0)
      res.set_res("dokie");
    else
      res.set_res("pokie");
  }
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
  EXPECT_EQ(scf->service_count(), 2);

  test_proto::subscribe_data_in in;
  test_proto::subscribe_data_out out;
  
  in.set_str(std::string("okie")); 

  test_service_one_client::shptr tso
    =  scf->subscribe_service<test_service_one_client>("test_service_one", &in, &out);
  ASSERT_TRUE(tso.get() != NULL);

  EXPECT_EQ(out.str(), "dokie");

  EXPECT_TRUE( scf->has_service_with_name("basic_protocol"));
  
  test_proto::Request   req;
  test_proto::Response  res;
  
  req.set_req("okie");
  tso->dummy_rpc(req,res);

  EXPECT_EQ(res.res(), "dokie");

  test_service_one_client::shptr tso2
    =  scf->get_service<test_service_one_client>("test_service_one");

  req.Clear();
  res.Clear();
  
  req.set_req("okie");
  tso2->dummy_rpc(req,res);

  EXPECT_EQ(res.res(), "dokie");

  scf->unsubscribe_service("test_service_one");
  
  ASSERT_FALSE(tso->is_subscribed());
  ASSERT_FALSE(tso2->is_subscribed());
}

#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
  ::testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
#endif
