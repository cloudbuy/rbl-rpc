#include <gtest/gtest.h>
#include <rpc/client/ClientServiceFactory.h>
#include <rpc/backend/BackEndBase.h>
#include <rpc/invoker/InProcessInvoker.h>
#include <rpc/backend/ClientServiceCookies.h>
#include <rpc/proto/TestService-server.rblrpc.h>
#include <rpc/proto/TestService-client.rblrpc.h>
#include <rpc/frontend/TcpFrontEnd.h>
#include <rpc/invoker/TcpInvoker.h>

using namespace rubble::rpc;

TEST(temp, temp_test)
{
  BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
  b.pool_size(1);
  b.start();
  TcpFrontEnd tfe(b,5555);
 
  tfe.start_accept();
 
  TcpInvoker inv("127.0.0.1", 5555);
  EXPECT_TRUE( inv.is_useable());
  
  TcpInvoker inv2("127.0.0.1", 7777);
  EXPECT_FALSE( inv2.is_useable());
}

#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
  ::testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
#endif
