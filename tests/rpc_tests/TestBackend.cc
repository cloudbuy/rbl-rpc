#include <gtest/gtest.h>
#include <rpc/server/backend/backend.h>

using namespace rubble::rpc;

TEST(BackendInitialization, all)
{
  BackEnd b;
  b.pool_size(2);
  
  EXPECT_EQ(b.pool_size(), 2);
  b.start(); 
  //b.shutdown();
  b.block_till_termination();
}

#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
#endif
