#include <gtest/gtest.h>
#include <rpc/client/ClientServiceFactory.h>
#include <rpc/invoker/InProcessInvoker.h>

TEST(client_factory, compilation_test)
{
}

#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
  ::testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
#endif
