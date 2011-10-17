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

TEST(DisconectionTests, test_one)
{
  TcpInvoker::scptr ti;
  TcpInvoker::scptr ti2;
  BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
  b.pool_size(1);

  b.start();

  TcpFrontEnd tfe(b,5555);
  tfe.start();


  // since handle accept is asynchronous the following operations can fail.
  // to prevent the failure the sleep time may be increased.
  
  ti.reset(new TcpInvoker("127.0.0.1", 5555));
  EXPECT_TRUE(ti->m_socket.is_open());
  boost::this_thread::sleep(boost::posix_time::seconds(1));
  EXPECT_EQ(tfe.connection_count(),1); 

  ti2.reset(new TcpInvoker("127.0.0.1", 5555));
  EXPECT_TRUE(ti2->m_socket.is_open());
  boost::this_thread::sleep(boost::posix_time::seconds(1));
  EXPECT_EQ(tfe.connection_count(),2); 
  ASSERT_EQ(b.client_count(),2);

  ASSERT_EQ(tfe.rpc_count(), 0);

  ASSERT_TRUE(ti->is_useable());
  //std::cin.get();     
}

#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
  ::testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
#endif
