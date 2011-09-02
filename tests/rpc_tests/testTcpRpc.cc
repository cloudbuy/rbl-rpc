//#define  BOOST_ASIO_ENABLE_HANDLER_TRACKING
#include <gtest/gtest.h>
#include <rpc/client/ClientServiceFactory.h>
#include <rpc/backend/BackEndBase.h>
#include <rpc/invoker/InProcessInvoker.h>
#include <rpc/backend/ClientServiceCookies.h>
#include <rpc/proto/TestService-server.rblrpc.h>
#include <rpc/proto/TestService-client.rblrpc.h>
#include <rpc/frontend/TcpFrontEnd.h>
#include <rpc/invoker/TcpInvoker.h>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace rubble::rpc;

TEST(temp, temp_test)
{
  BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
  b.pool_size(1);
  b.start();
  TcpFrontEnd tfe(b,5555);
 
  tfe.start();
 
  TcpInvoker inv("127.0.0.1", 5555);
  EXPECT_TRUE( inv.is_useable());
  
  TcpInvoker inv2("127.0.0.1", 7777);
  EXPECT_FALSE( inv2.is_useable());

  basic_protocol::HelloRequest hello;
  basic_protocol::HelloResponse hres;

  inv.reset();
  hello.set_source_type( basic_protocol::SOURCE_RELAY);
  hello.set_expected_target( basic_protocol::TARGET_MARSHALL); 
  hello.set_node_name("test_client");

  inv.client_data()->request().Clear(); 
  inv.client_data()->request().set_service_ordinal(0);
  inv.client_data()->request().set_request_ordinal(0);
  hello.SerializeToString(inv.client_data()->request().mutable_request_string());

  inv.invoke();
}

#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
  ::testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
#endif
