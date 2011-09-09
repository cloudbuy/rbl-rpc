#include <gtest/gtest.h>
#include <rpc/backend/BackEndBase.h>
#include <rpc/invoker/InProcessInvoker.h>
#include <rpc/backend/ClientServiceCookies.h>
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

TEST(backed_tests,duplicate_identifier_fail)
{
  BackEnd b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL);
  ServiceBase::shp s(new test_service_one<test_service_one_no_fail>());
  b.register_and_init_service(s);

  ASSERT_THROW( b.register_and_init_service(s),BackEndException);
}

TEST(backed_tests,service_innit_fail)
{
  BackEnd b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL);
  ServiceBase::shp s(new test_service_one<test_service_one_fail>());
  
  ASSERT_THROW( b.register_and_init_service(s),BackEndException);

}

TEST(backed_tests,teardown_no_fail)
{
  BackEnd b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL);
  ServiceBase::shp s(new test_service_one<test_service_one_dest_no_fail>());
  b.register_and_init_service(s);

  
  b.pool_size(1);
  b.start();
  ASSERT_NO_THROW(b.shutdown());
}

TEST(backed_tests,teardown_fail)
{
  BackEnd b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL);
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
    : b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL),
      s(new test_service_one<test_service_one_impl>()),
      invoker(b) {}
  protected:
  void SetUp() 
  {
    b.register_and_init_service(s);
    b.pool_size(1);
    b.start();
  }
  virtual void TearDown()
  {
  } 

  void hello_invoke()
  {
    invoker.reset();
    hello.set_source_type( source);
    hello.set_expected_target( destination); 
    hello.set_node_name("test_client");
  
    invoker.client_data()->request().Clear(); 
    invoker.client_data()->request().set_service_ordinal(0);
    invoker.client_data()->request().set_request_ordinal(0);
    hello.SerializeToString(invoker.client_data()->request().mutable_request_string());
    b.invoke(invoker);
    hres.ParseFromString(invoker.client_data()->response().response_string());
  }  
  void set_client_source(basic_protocol::SourceConnectionType s_in)
    { source = s_in; }
  void set_client_destination(basic_protocol::DestinationConnectionType d_in)
    { destination = d_in; }

  basic_protocol::SourceConnectionType        source;
  basic_protocol::DestinationConnectionType   destination;

  BackEnd b;
  ServiceBase::shp s;
  InProcessInvoker  invoker;
  basic_protocol::HelloRequest hello;
  basic_protocol::HelloResponse hres;
};

TEST_F(HelloTest, connect_correct_hello)
{
  EXPECT_EQ( invoker.client_data().use_count(), 1);

  set_client_source(basic_protocol::SOURCE_RELAY);
  set_client_destination(basic_protocol::TARGET_MARSHALL);

  ASSERT_FALSE(invoker.client_data()->is_rpc_active()); 
    
  EXPECT_EQ(invoker.client_data()->name(), "");

  hello_invoke();
  
  EXPECT_EQ( invoker.client_data().use_count(), 1);
    
  EXPECT_FALSE(invoker.client_data()->is_rpc_active());
  EXPECT_EQ(invoker.client_data()->name() , "test_client");

  EXPECT_EQ(hres.error_type(), basic_protocol::NO_HELLO_ERRORS);
  EXPECT_FALSE(invoker.client_data()->error_code());
  EXPECT_FALSE(invoker.client_data()->should_disconect() );
  EXPECT_TRUE(invoker.client_data()->is_client_established());
}

TEST_F(HelloTest, connect_multiple_hello)
{
  set_client_source(basic_protocol::SOURCE_RELAY);
  set_client_destination(basic_protocol::TARGET_MARSHALL);

  ASSERT_FALSE(invoker.client_data()->is_rpc_active()); 
    
  EXPECT_EQ(invoker.client_data()->name(), "");

  hello_invoke();
  hello_invoke();
  EXPECT_FALSE(invoker.client_data()->is_rpc_active());
//  EXPECT_EQ(hres.error_type(), basic_protocol::NO_HELLO_ERRORS);
  EXPECT_TRUE(invoker.client_data()->error_code());
  EXPECT_EQ(invoker.client_data()->error_code().value(), error_codes::RBL_BACKEND_ALLREADY_ESTABLISHED);

  EXPECT_TRUE(invoker.client_data()->should_disconect() );
  EXPECT_TRUE(invoker.client_data()->is_client_established());
  EXPECT_EQ(hres.error_type(), basic_protocol::CLIENT_ALLREADY_ESTABLISHED);
}

TEST_F(HelloTest, connect_incorect_source_hello)
{
  set_client_source(basic_protocol::SOURCE_GENERATOR);
  set_client_destination(basic_protocol::TARGET_MARSHALL);

  hello_invoke();

  EXPECT_EQ(hres.error_type(), basic_protocol::SOURCE_EXPECTATION_MISMATCH);
  EXPECT_EQ(invoker.client_data()->error_code().value(), error_codes::RBL_BACKEND_CLIENT_SOURCE_TYPE_MISMATCH);
  EXPECT_TRUE(invoker.client_data()->should_disconect() );
  EXPECT_FALSE(invoker.client_data()->is_client_established());
}

TEST_F(HelloTest, connect_incorect_destination_hello)
{
  set_client_source(basic_protocol::SOURCE_RELAY);
  set_client_destination(basic_protocol::TARGET_RELAY);

  hello_invoke();

  EXPECT_EQ(hres.error_type(), basic_protocol::DESTINATION_EXPECTATION_MISMATCH);
  EXPECT_EQ(invoker.client_data()->error_code().value(), error_codes::RBL_BACKEND_CLIENT_TARGET_TYPE_MISMATCH);
  EXPECT_TRUE(invoker.client_data()->should_disconect() );
  EXPECT_FALSE(invoker.client_data()->is_client_established());

}

class MissingIdTest : public ::testing::Test
{
public:
  MissingIdTest()
    : b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL),
      s(new test_service_one<test_service_one_impl>()),
      invoker(b) 
  {
  }
protected:

  virtual void SetUp() 
  {
    source = basic_protocol::SOURCE_RELAY;
    destination = basic_protocol::TARGET_MARSHALL;

    b.register_and_init_service(s);
    b.pool_size(1);
    b.start();
 
    hello.set_source_type( source);
    hello.set_expected_target( destination); 
    hello.set_node_name("test_client");
  
    invoker.client_data()->request().Clear(); 
    invoker.client_data()->request().set_service_ordinal(0);
    invoker.client_data()->request().set_request_ordinal(0);
    hello.SerializeToString(invoker.client_data()->request().mutable_request_string());
    b.invoke(invoker);
    hres.ParseFromString(invoker.client_data()->response().response_string());
  }
  
  void method_test()
  {
    invoker.reset();
    invoker.client_data()->request().Clear();
    invoker.client_data()->request().set_service_ordinal(1);
    invoker.client_data()->request().set_request_ordinal(5);
    b.invoke(invoker);

  }

  void service_test()  
  {
    invoker.reset();
    invoker.client_data()->request().Clear();
    invoker.client_data()->request().set_service_ordinal(3);
    invoker.client_data()->request().set_request_ordinal(0);
    b.invoke(invoker);
  }
 
  virtual void TearDown()
  {
    
  } 

  basic_protocol::SourceConnectionType        source;
  basic_protocol::DestinationConnectionType   destination;

  BackEnd b;
  ServiceBase::shp s;
  ;
  InProcessInvoker invoker;
  basic_protocol::HelloRequest hello;
  basic_protocol::HelloResponse hres;
  
  basic_protocol::ListServicesRequest list;
  basic_protocol::ListServicesResponse lres;
};

TEST_F(MissingIdTest, missing_service_test)
{
  service_test();
  EXPECT_EQ(invoker.client_data()->error_code().value(), error_codes::RBL_BACKEND_INVOKE_NO_SERVICE_WITH_ORDINAL_ERROR); 
}

TEST_F(MissingIdTest, missing_method_test)
{
  method_test();
  EXPECT_EQ(invoker.client_data()->error_code().value(), error_codes::RBL_BACKEND_INVOKE_NO_REQUEST_WITH_ORDINAL_ERROR); 

}

class ListTest : public ::testing::Test
{
public:
  ListTest()
    : b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL),
      s(new test_service_one<test_service_one_impl>()),
      invoker(b) {}
protected:

  virtual void SetUp() 
  {
    source = basic_protocol::SOURCE_RELAY;
    destination = basic_protocol::TARGET_MARSHALL;

    b.register_and_init_service(s);
    b.pool_size(1);
    b.start();
    
    hello.set_source_type( source);
    hello.set_expected_target( destination); 
    hello.set_node_name("test_client");
  
    invoker.client_data()->request().Clear(); 
    invoker.client_data()->request().set_service_ordinal(0);
    invoker.client_data()->request().set_request_ordinal(0);
    hello.SerializeToString(invoker.client_data()->request().mutable_request_string());
    b.invoke(invoker);
    hres.ParseFromString(invoker.client_data()->response().response_string());
  }
  
  void list_invoke()
  {
    invoker.reset();
    invoker.client_data()->request().Clear();
    invoker.client_data()->request().set_service_ordinal(0);
    invoker.client_data()->request().set_request_ordinal(1);
    
    list.SerializeToString(invoker.client_data()->request().mutable_request_string());
    b.invoke(invoker);
    lres.ParseFromString(invoker.client_data()->response().response_string());
  }
  
  virtual void TearDown()
  {
  } 

  basic_protocol::SourceConnectionType        source;
  basic_protocol::DestinationConnectionType   destination;

  BackEnd b;
  ServiceBase::shp s;
  ;
  InProcessInvoker invoker;
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
      invoker(b) {}

  ~SubscribeTests()
  {
    //b.shutdown();
  }
protected:

  virtual void SetUp() 
  {
    source = basic_protocol::SOURCE_RELAY;
    destination = basic_protocol::TARGET_MARSHALL;

    b.register_and_init_service(s);
    b.pool_size(1);
    b.start();
    
    hello.set_source_type( source);
    hello.set_expected_target( destination); 
    hello.set_node_name("test_client");
  
    invoker.client_data()->request().Clear(); 
    invoker.client_data()->request().set_service_ordinal(0);
    invoker.client_data()->request().set_request_ordinal(0);
    hello.SerializeToString(invoker.client_data()->request().mutable_request_string());
    b.invoke(invoker);
    hres.ParseFromString(invoker.client_data()->response().response_string());
  }
  
  virtual void TearDown()
  {

  } 

 
  basic_protocol::SourceConnectionType        source;
  basic_protocol::DestinationConnectionType   destination;

  BackEnd b;
  InProcessInvoker invoker;
  ServiceBase::shp s;

  basic_protocol::HelloRequest hello;
  basic_protocol::HelloResponse hres;
};

TEST_F(SubscribeTests, unsubscribed_error)
{
  invoker.reset();
  invoker.client_data()->request().Clear(); 
  invoker.client_data()->request().set_service_ordinal(1);
  invoker.client_data()->request().set_request_ordinal(0);
  b.invoke(invoker);
  EXPECT_EQ(invoker.client_data()->error_code().value(), error_codes::RBL_BACKEND_INVOKE_CLIENT_NOT_SUBSCRIBED);
}

TEST_F(SubscribeTests, subscribe_out_of_range_error)
{
  basic_protocol::SubscribeServiceRequest req;
  basic_protocol::SubscribeServiceResponse res;

  invoker.reset();

  req.set_service_ordinal(2);

  invoker.client_data()->request().Clear();
  invoker.client_data()->request().set_service_ordinal(0);
  invoker.client_data()->request().set_request_ordinal(2);
  req.SerializeToString(invoker.client_data()->request().mutable_request_string());
  
  b.invoke(invoker); 
  
  res.ParseFromString(invoker.client_data()->response().response_string());
  EXPECT_EQ(invoker.client_data()->error_code().value(),error_codes::RBL_BACKEND_SUBSCRIBE_NO_SERVICE_WITH_ORDINAL);
  EXPECT_EQ(res.error(), basic_protocol::SERVICE_ORDINAL_NOT_IN_USE);
}

TEST_F(SubscribeTests, subscribe_test)
{
  basic_protocol::SubscribeServiceRequest req;
  basic_protocol::SubscribeServiceResponse res;

  std::string in = "hahaha";
  std::string out = "QQ";
  
  invoker.reset();  

  req.set_service_ordinal(1);
  req.set_subscribe_request_string(in); 
  
  invoker.client_data()->request().Clear();
  invoker.client_data()->request().set_service_ordinal(0);
  invoker.client_data()->request().set_request_ordinal(2);
  req.SerializeToString(invoker.client_data()->request().mutable_request_string());
  
  b.invoke(invoker);
  
  EXPECT_FALSE(invoker.client_data()->error_code()) << invoker.client_data()->error_code().value();
  
  res.ParseFromString(invoker.client_data()->response().response_string());
  EXPECT_EQ(res.error(),basic_protocol::NO_SUBSCRIBE_SERVICE_ERROR);
  EXPECT_EQ(res.subscribe_result_string().compare("QQ"),0);
}


TEST_F(SubscribeTests, double_subscribe_test)
{
  
  basic_protocol::SubscribeServiceRequest req;
  basic_protocol::SubscribeServiceResponse res;

  std::string in = "hahaha";
  std::string out = "QQ";
  
  invoker.reset();  

  req.set_service_ordinal(1);
  req.set_subscribe_request_string(in); 
  
  invoker.client_data()->request().Clear();
  invoker.client_data()->request().set_service_ordinal(0);
  invoker.client_data()->request().set_request_ordinal(2);
  req.SerializeToString(invoker.client_data()->request().mutable_request_string());
  
  b.invoke(invoker);

  req.Clear();
  res.Clear();
 
  invoker.reset();  

  req.set_service_ordinal(1);
  req.set_subscribe_request_string(in); 
  
  invoker.client_data()->request().Clear();
  invoker.client_data()->request().set_service_ordinal(0);
  invoker.client_data()->request().set_request_ordinal(2);
  req.SerializeToString(invoker.client_data()->request().mutable_request_string());
  
  b.invoke(invoker);
 
  EXPECT_EQ(invoker.client_data()->error_code().value(),error_codes::RBL_BACKEND_ALLREADY_SUBSCRIBED);
  res.ParseFromString(invoker.client_data()->response().response_string());
  EXPECT_EQ(res.error(),basic_protocol::SERVICE_ALLREADY_SUBSCRIBED); 
}

class Cookie : public ClientCookieBase
{
public:
  void set_destructor_sentinel(bool * sent)
  {
    dest_sentinel = sent;
  }
  ~Cookie()
  {
    * dest_sentinel = true;
  }

  int magic_number;
  bool * dest_sentinel;
};

class test_service_cookie_test
{
public:
  typedef ClientCookieBase t_client_cookie;
  void init(boost::system::error_code & ec) { ec.clear(); }
  void subscribe(ClientCookie & client_cookie, ClientData & cd,
    std::string * in, std::string * out) 
  {
    Cookie * cookie = 0;
    client_cookie.create_cookie(&cookie);
    address = (void *) cookie;
    cookie->magic_number = 100;
    sentinel = false;
    cookie->set_destructor_sentinel(&sentinel);
    unsubscribe_sentinel = false;
  }
  void teardown(boost::system::error_code & ec) { ec.clear(); }
  void unsubscribe(ClientCookie & client_cookie, ClientData & cd) 
  {
    unsubscribe_sentinel = true;
  }
  void dummy_rpc(ClientCookie & client_cookie,ClientData &,Request & ,Response & )
  {
    Cookie * cookie;
    client_cookie.retrieve_cookie(&cookie);

    if(cookie == address)
      ca = true;
    else
      ca = false;
    
    if ( cookie->magic_number == 100)
      mn = true;
    else
      mn = false;
  }
  
  const bool compare_cookie_magic_number() const
  {
    return mn;
  }
  
  const bool compare_cookie_address() const
  {
    return ca; 
  } 

  void * address;
  bool ca;
  bool mn;
  bool sentinel;;
  bool unsubscribe_sentinel;
};

class CookieTest : public ::testing::Test
{
public:
  CookieTest()
    : b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL),
      s(new test_service_one<test_service_cookie_test>()),
      invoker(new InProcessInvoker(b)) {}
protected:

  virtual void SetUp() 
  {
    source = basic_protocol::SOURCE_RELAY;
    destination = basic_protocol::TARGET_MARSHALL;

    b.register_and_init_service(s);
    b.pool_size(1);
    b.start();
    
    hello.set_source_type( source);
    hello.set_expected_target( destination); 
    hello.set_node_name("test_client");
  
    invoker->client_data()->request().Clear(); 
    invoker->client_data()->request().set_service_ordinal(0);
    invoker->client_data()->request().set_request_ordinal(0);
    hello.SerializeToString(invoker->client_data()->request().mutable_request_string());
    b.invoke(*invoker);
    hres.ParseFromString(invoker->client_data()->response().response_string());
    
    basic_protocol::SubscribeServiceRequest req;
    basic_protocol::SubscribeServiceResponse res;

    std::string in = "hahaha";
    std::string out = "QQ";
  
    invoker->reset();  

    req.set_service_ordinal(1);
    req.set_subscribe_request_string(in); 
  
    invoker->client_data()->request().Clear();
    invoker->client_data()->request().set_service_ordinal(0);
    invoker->client_data()->request().set_request_ordinal(2);
    req.SerializeToString(invoker->client_data()->request().mutable_request_string());
    
    b.invoke(*invoker);
    
    invoker->reset();
    invoker->client_data()->request().Clear();
    invoker->client_data()->request().set_service_ordinal(1);
    invoker->client_data()->request().set_request_ordinal(0);
    
    b.invoke(*invoker);  

    test_service_one<test_service_cookie_test> * s_
      = static_cast<test_service_one<test_service_cookie_test> *>( s.get());
  
    s__ = &s_->get_impl();
  }
  
  
  virtual void TearDown()
  {
  } 

  basic_protocol::SourceConnectionType        source;
  basic_protocol::DestinationConnectionType   destination;

  BackEnd b;
  ServiceBase::shp s;
  ;
  boost::shared_ptr<InProcessInvoker> invoker;
  basic_protocol::HelloRequest hello;
  basic_protocol::HelloResponse hres;
  
  basic_protocol::ListServicesRequest list;
  basic_protocol::ListServicesResponse lres;
  
  test_service_one<test_service_cookie_test> * s_;
  const test_service_cookie_test * s__;
};

TEST_F(CookieTest, cookie_association_test)
{
  
  EXPECT_TRUE( s__->compare_cookie_magic_number()  );
  EXPECT_TRUE( s__->compare_cookie_address());
}

TEST_F(CookieTest, cookie_dc_test)
{

  const ClientServiceCookies & c_csc = static_cast<const BackEnd *>(&b)->cookies();
  
  EXPECT_FALSE( c_csc.contains_cookie(1,invoker->client_data().get()) == ClientServiceCookies::COOKIE_ABSENT);
  
  invoker->disconect_from_backend();
  
  EXPECT_TRUE(s__->unsubscribe_sentinel);
  EXPECT_TRUE(s__->sentinel);

  
  EXPECT_TRUE( c_csc.contains_cookie(1,invoker->client_data().get()) == ClientServiceCookies::COOKIE_ABSENT);
}

TEST_F(CookieTest, cookie_unsubscrive_test)
{
  basic_protocol::UnsubscribeServiceRequest us_req;
  basic_protocol::UnsubscribeServiceResponse us_res;
 
  invoker->reset();
 
  us_req.set_service_ordinal(1);
  
  invoker->client_data()->request().Clear();
  invoker->client_data()->request().set_service_ordinal(0);
  invoker->client_data()->request().set_request_ordinal(3);
  us_req.SerializeToString(invoker->client_data()->request().mutable_request_string());

  const ClientServiceCookies & c_csc = static_cast<const BackEnd *>(&b)->cookies();
  
  EXPECT_FALSE( c_csc.contains_cookie(1,invoker->client_data().get()) == ClientServiceCookies::COOKIE_ABSENT);
 

  b.invoke(*invoker); 

  EXPECT_TRUE( c_csc.contains_cookie(1,invoker->client_data().get()) == ClientServiceCookies::COOKIE_ABSENT);

  // now test get a new cookie for the same service, the fact that it should
  // result with a cookie in the uninitialized state.

  // the can be asserted from the fact that when trying to perform a valid 
  // rpc call, the result should be an error stating that the service 
  // has not been  mounted by this client.
  invoker->reset();
  
  invoker->client_data()->request().Clear();
  invoker->client_data()->request().set_service_ordinal(1);
  invoker->client_data()->request().set_request_ordinal(0);
    
  b.invoke(*invoker);  
  
  EXPECT_EQ(invoker->client_data()->error_code().value(), error_codes::RBL_BACKEND_INVOKE_CLIENT_NOT_SUBSCRIBED);
}

class ListMethodTest : public ::testing::Test
{
public:
  ListMethodTest()
    : b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL),
      s(new test_service_one<test_service_one_impl>()),
      invoker(b) {}
  protected:
  virtual void SetUp() 
  {
    b.register_and_init_service(s);
    b.pool_size(1);
    b.start();
  }
  virtual void TearDown()
  {
  } 

  void hello_invoke()
  {
    hello.set_source_type( source);
    hello.set_expected_target( destination); 
    hello.set_node_name("test_client");
  
    invoker.client_data()->request().Clear(); 
    invoker.client_data()->request().set_service_ordinal(0);
    invoker.client_data()->request().set_request_ordinal(0);
    hello.SerializeToString(invoker.client_data()->request().mutable_request_string());
    b.invoke(invoker);
    hres.ParseFromString(invoker.client_data()->response().response_string());
  }  
  void set_client_source(basic_protocol::SourceConnectionType s_in)
    { source = s_in; }
  void set_client_destination(basic_protocol::DestinationConnectionType d_in)
    { destination = d_in; }

  basic_protocol::SourceConnectionType        source;
  basic_protocol::DestinationConnectionType   destination;

  BackEnd b;
  ServiceBase::shp s;
  
  InProcessInvoker invoker;
  basic_protocol::HelloRequest hello;
  basic_protocol::HelloResponse hres;
};

//test the top 4 messages
TEST_F(ListMethodTest, basic_protocol_messages)
{
  EXPECT_TRUE(b.is_useable());
  basic_protocol::ListMethodsRequest req;
  basic_protocol::ListMethodsResponse res;

  req.set_service_ordinal(0);

  invoker.reset();
  invoker.client_data()->request().Clear(); 
  invoker.client_data()->request().set_service_ordinal(0);
  invoker.client_data()->request().set_request_ordinal(4);

  req.SerializeToString(invoker.client_data()->request().mutable_request_string());
  
  b.invoke(invoker);
  res.ParseFromString(invoker.client_data()->response().response_string());

  EXPECT_TRUE(res.methods_size() >= 5);
  EXPECT_EQ( res.error(), 0);
  EXPECT_EQ( res.methods(0).service_name(),"hello" );
  EXPECT_EQ( res.methods(1).service_name(),"list_services"); 
  EXPECT_EQ( res.methods(2).service_name(),"rpc_subscribe_service");
  EXPECT_EQ( res.methods(3).service_name(),"rpc_unsubscribe_service");
  EXPECT_EQ( res.methods(4).service_name(),"list_methods");
}

class NotAcceptingTests : public ::testing::Test
{
public:
  NotAcceptingTests()
    : b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL),
      s(new test_service_one<test_service_one_impl>()),
      invoker(b) {}
  protected:
  virtual void SetUp() 
  {
    b.register_and_init_service(s);
    b.pool_size(1);
  }
  virtual void TearDown()
  {
  } 

  void hello_invoke()
  {
    set_client_source(basic_protocol::SOURCE_RELAY);
    set_client_destination(basic_protocol::TARGET_MARSHALL);

    ASSERT_FALSE(invoker.client_data()->is_rpc_active()); 
    
    EXPECT_EQ(invoker.client_data()->name(), "");

    invoker.reset();
    hello.set_source_type( source);
    hello.set_expected_target( destination); 
    hello.set_node_name("test_client");
  
    invoker.client_data()->request().Clear(); 
    invoker.client_data()->request().set_service_ordinal(0);
    invoker.client_data()->request().set_request_ordinal(0);
    hello.SerializeToString(invoker.client_data()->request().mutable_request_string());
    b.invoke(invoker);
  }  
  void set_client_source(basic_protocol::SourceConnectionType s_in)
    { source = s_in; }
  void set_client_destination(basic_protocol::DestinationConnectionType d_in)
    { destination = d_in; }

  basic_protocol::SourceConnectionType        source;
  basic_protocol::DestinationConnectionType   destination;

  BackEnd b;
  ServiceBase::shp s;
  InProcessInvoker  invoker;
  basic_protocol::HelloRequest hello;
  basic_protocol::HelloResponse hres;
};

TEST_F(NotAcceptingTests, before_backend_start)
{
  hello_invoke();
  EXPECT_EQ(invoker.client_data()->response().error(), basic_protocol::NOT_ACCEPTING_REQUESTS);
  EXPECT_EQ(invoker.client_data()->error_code().value(), error_codes::RBL_BACKEND_NOT_ACCEPTING_REQUESTS);
  EXPECT_FALSE(b.is_useable());
}

TEST_F(NotAcceptingTests, after_shutdown_test)
{
  b.start();
  b.shutdown(); 
  hello_invoke();
  
  EXPECT_EQ(invoker.client_data()->response().error(), basic_protocol::NOT_ACCEPTING_REQUESTS);
  EXPECT_EQ(invoker.client_data()->error_code().value(), error_codes::RBL_BACKEND_NOT_ACCEPTING_REQUESTS);
  EXPECT_FALSE(b.is_useable());
}

#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}

TEST(invoker_backend_register_tests, in_process_invoker_disconect_test)
{
  BackEnd b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL);
  ServiceBase::shp s(new test_service_one<test_service_one_dest_no_fail>());
  b.register_and_init_service(s);

  
  b.pool_size(1);
  b.start();
  
  EXPECT_EQ(b.client_count(), 0);
  InProcessInvoker::shptr inv(new InProcessInvoker(b));
  EXPECT_EQ(b.client_count(),1);
  InProcessInvoker::shptr inv2(new InProcessInvoker(b));
  EXPECT_EQ(b.client_count(),2);
  inv2.reset();
  EXPECT_EQ(b.client_count(),1);
  inv.reset();
  EXPECT_EQ(b.client_count(),0);
}

TEST(invoker_backend_register_tests, backend_invoker_disconect_test)
{
  BackEnd::shptr b( new BackEnd(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL));
  ServiceBase::shp s(new test_service_one<test_service_one_dest_no_fail>());
  b->register_and_init_service(s);

  
  b->pool_size(1);
  b->start();
  
  EXPECT_EQ(b->client_count(), 0);
  InProcessInvoker::shptr inv(new InProcessInvoker(*b));
  InProcessInvoker::shptr inv2(new InProcessInvoker(*b));
  
  b.reset();
  EXPECT_FALSE(inv->is_connected());
  EXPECT_FALSE(inv->is_connected());
}

  class test_service_delayed_rpc
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.clear(); }
    void teardown(boost::system::error_code & ec) {ec.clear();}
    void subscribe(ClientCookie & client_cookie, ClientData & cd,
      std::string *, std::string *) {}
    void unsubscribe(ClientCookie & client_cookie, ClientData & cd) {}

    void dummy_rpc(ClientCookie &,ClientData &,Request & ,Response & )
    {
      notify.wait_for_notification();
//      boost::this_thread::sleep(boost::posix_time::seconds(2));
    }

    NotificationObject notify; 
  };

struct in_thread_invoker
{
  void operator()(BackEnd::shptr b,InProcessInvoker::shptr inv)
  {
    InProcessInvoker & ipi = *inv;
    b->invoke(ipi);
  }
};

TEST(invoker_backend_register_tests, backend_invoker_active_rpc)
{
  BackEnd::shptr b( new BackEnd(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL));
  test_service_one<test_service_delayed_rpc> * s_p 
    = new test_service_one<test_service_delayed_rpc>();
  ServiceBase::shp s(s_p);
  b->register_and_init_service(s);
  
  b->pool_size(1);
  b->start();
 
  basic_protocol::SubscribeServiceRequest sreq;
  basic_protocol::SubscribeServiceResponse sres;
 
  EXPECT_EQ(b->client_count(), 0);
  InProcessInvoker::shptr inv(new InProcessInvoker(*b));

  inv->reset();  
  sreq.set_service_ordinal(1);

  inv->client_data()->request().Clear();
  inv->client_data()->request().set_service_ordinal(0);
  inv->client_data()->request().set_request_ordinal(2);
  sreq.SerializeToString(inv->client_data()->request().mutable_request_string());
  
  b->invoke(*inv);
  
  EXPECT_FALSE(inv->client_data()->error_code()) << inv->client_data()->error_code().value();
  
  sres.ParseFromString(inv->client_data()->response().response_string());
  EXPECT_EQ(sres.error(),basic_protocol::NO_SUBSCRIBE_SERVICE_ERROR);


  inv->client_data()->request().Clear();
  inv->client_data()->request().set_service_ordinal(1);
  inv->client_data()->request().set_request_ordinal(0);

  test_proto::Request req;
  test_proto::Response res;  

  req.SerializeToString(inv->client_data()->request().mutable_request_string());
  boost::thread t(boost::bind<void>(in_thread_invoker(),b,inv));
  boost::thread t1(boost::bind(&BackEnd::shutdown,b,5));
  s_p->impl().notify.notify_one();
  t1.join();
}



#endif
