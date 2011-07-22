#include <gtest/gtest.h>
#include <rpc/server/ClientServiceCookies.h>

using namespace rubble::rpc;

class TestClass : public ClientCookieBase
{
public:
  bool sentinel;
  const int int_ =1;
  ~TestClass() { sentinel = true;}
};

class TestClass2 : public ClientCookieBase
{
public:
  bool sentinel;
  const int int_ = 2;
  ~TestClass2() { sentinel = true; }
};

TEST(ClientServiceCookies, all)
{
  ClientData * cd1 = new ClientData();
  ClientData * cd2 = new ClientData();

  ClientServiceCookies csc;
  csc.set_size(5);
  
  csc.activate_service_with_ordinal(0);
  csc.activate_service_with_ordinal(1);

  std:size_t sz = 0;
  csc.cookie_count(0, sz);
  EXPECT_EQ(sz, 0); 
  
  sz = 0;
  csc.cookie_count(1, sz);
  EXPECT_EQ(sz, 0); 

  ClientCookie *cc1;
  ClientCookie *cc2;

  csc.create_or_retrieve_cookie(0,cd1, &cc1);
  
  sz = 0;
  csc.cookie_count(0, sz);
  EXPECT_EQ(sz, 1); 
  
  csc.create_or_retrieve_cookie(0,cd2, &cc2);
  
  sz = 0;
  csc.cookie_count(0, sz);
  EXPECT_EQ(sz, 2); 

  TestClass   * tc_1 = 0;
  TestClass * tc_2 =0;
 
 
  cc1->create_cookie(&tc_1);
  EXPECT_TRUE(tc_1!= 0);
  //cc1.create_cookie(&tc); //<--- should trigger assert
  
  cc1->retrieve_cookie(&tc_2);
  ASSERT_EQ(tc_1, tc_2);
  //cc2.retrieve_cookie(&tc2); //<-- should trigger assert


  ClientCookie * cc1_2;
  TestClass * tc_3 = NULL;

  csc.create_or_retrieve_cookie(0,cd1,&cc1_2);
  sz = 0;
  csc.cookie_count(0, sz);
  EXPECT_EQ(sz, 2); 

  cc1_2->retrieve_cookie(&tc_3);

  ASSERT_EQ(tc_1, tc_3);

  tc_3->sentinel=false;

  bool * sent=&tc_3->sentinel;
  EXPECT_FALSE( * sent);

  ASSERT_TRUE(csc.delete_cookie(0,cd1)!=ClientServiceCookies::COOKIE_ABSENT);
  sz = 0;
  csc.cookie_count(0, sz);
  EXPECT_EQ(sz, 1); 

  ASSERT_TRUE(csc.delete_cookie(0,cd1)==ClientServiceCookies::COOKIE_ABSENT);
  
  sz = 0;
  csc.cookie_count(0, sz);
  EXPECT_EQ(sz, 1); 

  EXPECT_TRUE(*sent); 

  //cc1_2->retrieve_cookie(&tc_3); //<- should trigger asser
  //
  TestClass2 *tc2;
  cc2->create_cookie(&tc2); 

  tc2->sentinel = false;
  bool * sentinel2 = &tc2->sentinel;
  cc2->destroy_cookie();
  ASSERT_TRUE(*sentinel2);
  
}

#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
#endif
