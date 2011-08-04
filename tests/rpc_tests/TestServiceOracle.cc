#include <gtest/gtest.h>
#include <rpc/server/backend/LocalBackEnd.h>
#include <rpc/server/ClientServiceCookies.h>
#include <rpc/proto/TestService-server.rblrpc.h>

using namespace rubble::rpc;
using namespace rubble::rpc::test_proto;

  class test_service_one_no_fail
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.clear(); }
    void teardown(boost::system::error_code & ec) {}
    void subscribe(ClientCookie & client_cookie, ClientData & cd,
      std::string *, std::string *) {}
    void unsubscribe(ClientCookie & client_cookie, ClientData & cd) {}
    void dummy_rpc(ClientCookie &,ClientData &,Request & ,Response & ){}
  private:
  };

  class test_service_one_fail
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.assign(1,rpc_backend_error); }
    void teardown(boost::system::error_code & ec) {}
    void subscribe(ClientCookie & client_cookie, ClientData & cd,
      std::string *, std::string *) {}
    void unsubscribe(ClientCookie & client_cookie, ClientData & cd) {}
    void dummy_rpc(ClientCookie &,ClientData &,Request & ,Response & ){}
  private:
  };

  class test_service_one_dest_no_fail
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.clear(); }
    void teardown(boost::system::error_code & ec) {ec.clear();}
    void subscribe(ClientCookie & client_cookie, ClientData & cd,
      std::string *, std::string *) {}
    void unsubscribe(ClientCookie & client_cookie, ClientData & cd) {}
    void dummy_rpc(ClientCookie &,ClientData &,Request & ,Response & ){}
  private:
  };

  class test_service_one_dest_fail
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.clear(); }
    void teardown(boost::system::error_code & ec) { ec.assign(1,rpc_backend_error); }
    void subscribe(ClientCookie & client_cookie, ClientData & cd,
      std::string *, std::string *) {}
    void unsubscribe(ClientCookie & client_cookie, ClientData & cd) {}
    void dummy_rpc(ClientCookie &,ClientData &,Request & ,Response & ){}
  private:
  };

  class test_service_one_impl
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.clear(); }
    void subscribe(ClientCookie & client_cookie, ClientData & cd,
      std::string *, std::string *) {}
    void teardown(boost::system::error_code & ec) { ec.clear(); }
    void unsubscribe(ClientCookie & client_cookie, ClientData & cd) {}
    void dummy_rpc(ClientCookie &,ClientData &,Request & ,Response & ){}
  private:
  };

TEST(backed_tests,duplicate_identifier_fail)
{
  LocalBackEnd b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL);
  ServiceBase::shp s(new test_service_one<test_service_one_no_fail>());
  b.register_and_init_service(s);

  ASSERT_THROW( b.register_and_init_service(s),BackEndException);
}

TEST(backed_tests,service_innit_fail)
{
  LocalBackEnd b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL);
  ServiceBase::shp s(new test_service_one<test_service_one_fail>());
  
  ASSERT_THROW( b.register_and_init_service(s),BackEndException);

}

TEST(backed_tests,teardown_no_fail)
{
  LocalBackEnd b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL);
  ServiceBase::shp s(new test_service_one<test_service_one_dest_no_fail>());
  b.register_and_init_service(s);
  
  b.pool_size(1);
  b.start();
  ASSERT_NO_THROW(b.shutdown());
}

TEST(backed_tests,teardown_fail)
{
  LocalBackEnd b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL);
  ServiceBase::shp s(new test_service_one<test_service_one_dest_fail>());
  b.register_and_init_service(s);
  
  b.pool_size(1);
  b.start();
  ASSERT_THROW(b.shutdown(),BackEndException);
}

class HelloTest : public ::testing::Test
{
public:
  HelloTest()
    : b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL),
      s(new test_service_one<test_service_one_impl>()),
      cd((new ClientData())),
      invoker(cd) {}
  protected:
  virtual void SetUp() 
  {
    b.register_and_init_service(s);
    b.pool_size(1);
    b.start();
    
    b.connect(cd);
  }
  virtual void TearDown()
  {
    b.shutdown();
  } 

  void hello_invoke()
  {
    hello.set_source_type( source);
    hello.set_expected_target( destination); 
    hello.set_node_name("test_client");
  
    cd->request().Clear(); 
    cd->request().set_service_ordinal(0);
    cd->request().set_request_ordinal(0);
    hello.SerializeToString(cd->request().mutable_request_string());
    b.invoke(invoker);
    hres.ParseFromString(cd->response().response_string());
  }  
  void set_client_source(basic_protocol::SourceConnectionType s_in)
    { source = s_in; }
  void set_client_destination(basic_protocol::DestinationConnectionType d_in)
    { destination = d_in; }

  basic_protocol::SourceConnectionType        source;
  basic_protocol::DestinationConnectionType   destination;

  LocalBackEnd b;
  ServiceBase::shp s;
  ClientData::shp cd;
  LocalBackEnd::Invoker invoker;
  basic_protocol::HelloRequest hello;
  basic_protocol::HelloResponse hres;
};

TEST_F(HelloTest, connect_correct_hello)
{
  set_client_source(basic_protocol::SOURCE_RELAY);
  set_client_destination(basic_protocol::TARGET_MARSHALL);

  ASSERT_FALSE(cd->is_rpc_active()); 
    
  EXPECT_EQ(cd->name(), "");

  hello_invoke();

  EXPECT_EQ(cd->name() , "test_client");

  EXPECT_EQ(hres.error_type(), basic_protocol::NO_HELLO_ERRORS);
  EXPECT_FALSE(cd->error_code());
  EXPECT_FALSE(cd->should_disconect() );
  EXPECT_TRUE(cd->is_client_established());
}

TEST_F(HelloTest, connect_multiple_hello)
{
  set_client_source(basic_protocol::SOURCE_RELAY);
  set_client_destination(basic_protocol::TARGET_MARSHALL);

  ASSERT_FALSE(cd->is_rpc_active()); 
    
  EXPECT_EQ(cd->name(), "");

  hello_invoke();
  hello_invoke();

//  EXPECT_EQ(hres.error_type(), basic_protocol::NO_HELLO_ERRORS);
  EXPECT_TRUE(cd->error_code());
  EXPECT_EQ(cd->error_code().value(), error_codes::RBL_BACKEND_ALLREADY_ESTABLISHED);

  EXPECT_TRUE(cd->should_disconect() );
  EXPECT_TRUE(cd->is_client_established());
  EXPECT_EQ(hres.error_type(), basic_protocol::CLIENT_ALLREADY_ESTABLISHED);
}

TEST_F(HelloTest, connect_incorect_source_hello)
{
  set_client_source(basic_protocol::SOURCE_GENERATOR);
  set_client_destination(basic_protocol::TARGET_MARSHALL);

  hello_invoke();

  EXPECT_EQ(hres.error_type(), basic_protocol::SOURCE_EXPECTATION_MISMATCH);
  EXPECT_EQ(cd->error_code().value(), error_codes::RBL_BACKEND_CLIENT_SOURCE_TYPE_MISMATCH);
  EXPECT_TRUE(cd->should_disconect() );
  EXPECT_FALSE(cd->is_client_established());
}

TEST_F(HelloTest, connect_incorect_destination_hello)
{
  set_client_source(basic_protocol::SOURCE_RELAY);
  set_client_destination(basic_protocol::TARGET_RELAY);

  hello_invoke();

  EXPECT_EQ(hres.error_type(), basic_protocol::DESTINATION_EXPECTATION_MISMATCH);
  EXPECT_EQ(cd->error_code().value(), error_codes::RBL_BACKEND_CLIENT_TARGET_TYPE_MISMATCH);
  EXPECT_TRUE(cd->should_disconect() );
  EXPECT_FALSE(cd->is_client_established());

}

class MissingIdTest : public ::testing::Test
{
public:
  MissingIdTest()
    : b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL),
      s(new test_service_one<test_service_one_impl>()),
      cd((new ClientData())),
      invoker(cd) {}
protected:

  virtual void SetUp() 
  {
    source = basic_protocol::SOURCE_RELAY;
    destination = basic_protocol::TARGET_MARSHALL;

    b.register_and_init_service(s);
    b.pool_size(1);
    b.start();
    
    b.connect(cd);
  
    hello.set_source_type( source);
    hello.set_expected_target( destination); 
    hello.set_node_name("test_client");
  
    cd->request().Clear(); 
    cd->request().set_service_ordinal(0);
    cd->request().set_request_ordinal(0);
    hello.SerializeToString(cd->request().mutable_request_string());
    b.invoke(invoker);
    hres.ParseFromString(cd->response().response_string());
  }
  
  void method_test()
  {
    invoker.reset();
    cd->request().Clear();
    cd->request().set_service_ordinal(1);
    cd->request().set_request_ordinal(5);
    b.invoke(invoker);

  }

  void service_test()  
  {
    invoker.reset();
    cd->request().Clear();
    cd->request().set_service_ordinal(3);
    cd->request().set_request_ordinal(0);
    b.invoke(invoker);
  }
 
  virtual void TearDown()
  {
    b.shutdown();
  } 

  basic_protocol::SourceConnectionType        source;
  basic_protocol::DestinationConnectionType   destination;

  LocalBackEnd b;
  ServiceBase::shp s;
  ClientData::shp cd;
  LocalBackEnd::Invoker invoker;
  basic_protocol::HelloRequest hello;
  basic_protocol::HelloResponse hres;
  
  basic_protocol::ListServicesRequest list;
  basic_protocol::ListServicesResponse lres;
};

TEST_F(MissingIdTest, missing_service_test)
{
  service_test();
  EXPECT_EQ(cd->error_code().value(), error_codes::RBL_BACKEND_INVOKE_NO_SERVICE_WITH_ORDINAL_ERROR); 
}

TEST_F(MissingIdTest, missing_method_test)
{
  method_test();
  EXPECT_EQ(cd->error_code().value(), error_codes::RBL_BACKEND_INVOKE_NO_REQUEST_WITH_ORDINAL_ERROR); 

}

class ListTest : public ::testing::Test
{
public:
  ListTest()
    : b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL),
      s(new test_service_one<test_service_one_impl>()),
      cd((new ClientData())),
      invoker(cd) {}
protected:

  virtual void SetUp() 
  {
    source = basic_protocol::SOURCE_RELAY;
    destination = basic_protocol::TARGET_MARSHALL;

    b.register_and_init_service(s);
    b.pool_size(1);
    b.start();
    
    b.connect(cd);
  
    hello.set_source_type( source);
    hello.set_expected_target( destination); 
    hello.set_node_name("test_client");
  
    cd->request().Clear(); 
    cd->request().set_service_ordinal(0);
    cd->request().set_request_ordinal(0);
    hello.SerializeToString(cd->request().mutable_request_string());
    b.invoke(invoker);
    hres.ParseFromString(cd->response().response_string());
  }
  
  void list_invoke()
  {
    invoker.reset();
    cd->request().Clear();
    cd->request().set_service_ordinal(0);
    cd->request().set_request_ordinal(1);
    
    list.SerializeToString(cd->request().mutable_request_string());
    b.invoke(invoker);
    lres.ParseFromString(cd->response().response_string());
  }
  
  virtual void TearDown()
  {
    b.shutdown();
  } 

  basic_protocol::SourceConnectionType        source;
  basic_protocol::DestinationConnectionType   destination;

  LocalBackEnd b;
  ServiceBase::shp s;
  ClientData::shp cd;
  LocalBackEnd::Invoker invoker;
  basic_protocol::HelloRequest hello;
  basic_protocol::HelloResponse hres;
  
  basic_protocol::ListServicesRequest list;
  basic_protocol::ListServicesResponse lres;
};

TEST_F(ListTest, first_test)
{
  list_invoke();

  EXPECT_EQ(lres.services_size(),2);
  
  const basic_protocol::ServiceEntry & ser_1
    = lres.services(0);
  const basic_protocol::ServiceEntry & ser_2
    = lres.services(1);
  
  EXPECT_EQ(ser_1.service_ordinal(),0);
  EXPECT_EQ(ser_2.service_ordinal(),1);

  EXPECT_EQ(ser_1.service_name(),"basic_protocol");
  EXPECT_EQ(ser_2.service_name(),"test_service_one");
}

class SubscribeTests : public ::testing::Test
{
public:
  SubscribeTests()
    : b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL),
      s(new test_service_one<test_service_one_impl>()),
      cd((new ClientData())),
      invoker(cd) {}
protected:

  virtual void SetUp() 
  {
    source = basic_protocol::SOURCE_RELAY;
    destination = basic_protocol::TARGET_MARSHALL;

    b.register_and_init_service(s);
    b.pool_size(1);
    b.start();
    
    b.connect(cd);
  
    hello.set_source_type( source);
    hello.set_expected_target( destination); 
    hello.set_node_name("test_client");
  
    cd->request().Clear(); 
    cd->request().set_service_ordinal(0);
    cd->request().set_request_ordinal(0);
    hello.SerializeToString(cd->request().mutable_request_string());
    b.invoke(invoker);
    hres.ParseFromString(cd->response().response_string());
  }
  
  virtual void TearDown()
  {
    b.shutdown();
  } 

 
  basic_protocol::SourceConnectionType        source;
  basic_protocol::DestinationConnectionType   destination;

  LocalBackEnd b;
  ServiceBase::shp s;
  ClientData::shp cd;
  LocalBackEnd::Invoker invoker;
  basic_protocol::HelloRequest hello;
  basic_protocol::HelloResponse hres;
};

TEST_F(SubscribeTests, unsubscribed_error)
{
  invoker.reset();
  cd->request().Clear(); 
  cd->request().set_service_ordinal(1);
  cd->request().set_request_ordinal(0);
  b.invoke(invoker);
  EXPECT_EQ(cd->error_code().value(), error_codes::RBL_BACKEND_INVOKE_CLIENT_UNSUBSCRIBED);
}

TEST_F(SubscribeTests, subscribe_out_of_range_error)
{
  basic_protocol::SubscribeServiceRequest req;
  basic_protocol::SubscribeServiceResponse res;

  invoker.reset();

  req.set_service_ordinal(2);

  cd->request().Clear();
  cd->request().set_service_ordinal(0);
  cd->request().set_request_ordinal(2);
  req.SerializeToString(cd->request().mutable_request_string());
  

  b.invoke(invoker); 
  
  res.ParseFromString(cd->response().response_string());
  EXPECT_EQ(cd->error_code().value(),error_codes::RBL_BACKEND_SUBSCRIBE_NO_SERVICE_WITH_ORDINAL);
  EXPECT_EQ(res.error(), basic_protocol::SERVICE_ORDINAL_NOT_IN_USE);
}

#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
#endif
