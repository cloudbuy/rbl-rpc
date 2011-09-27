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
using namespace rubble::rpc::test_proto;

  class test_service_one_impl
  {
  public:
    typedef ClientCookieBase t_client_cookie;
    void init(boost::system::error_code & ec) { ec.clear(); }
    void subscribe(ClientCookie & client_cookie, ClientData & cd,
      std::string * in, std::string * out) 
    {
    }
    void teardown(boost::system::error_code & ec) { ec.clear(); }
    void unsubscribe(ClientCookie & client_cookie, ClientData & cd) {}
    void dummy_rpc(ClientCookie &,ClientData &,Request & ,Response & ){}
  private:
  };

TEST(RPC_EXCEPTION_TEST, IN_PROCESS_NOT_ACCEPTING)
{
  BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
  b.pool_size(1);

  ServiceBase::shp s(new test_service_one<test_service_one_impl>());
  b.register_and_init_service(s);
  b.start();
  ServiceClientFactory::scptr scf;
  InProcessInvoker ipi(b);  

  try 
  {
    scf.reset(new ServiceClientFactory( "test service", ipi,
                                        basic_protocol::SOURCE_RELAY, 
                                        basic_protocol::TARGET_MARSHALL));
    b.pause_requests();
    scf->subscribe_service<test_service_one_client>("test_service_one");
    FAIL() << "Exception should have been thrown";
  }
  catch(InvokerException ie)
  {
    boost::system::error_code * e = boost::get_error_info<rbl_invoker_error_code>(ie);
    
    EXPECT_EQ(e->value(), basic_protocol::NOT_ACCEPTING_REQUESTS);
    return;
  }
  catch(BackEndException b)
  {
    FAIL() << "should not throw a BackEndException";
  }
  catch(...)
  {
    FAIL() << "Wrong type of exception";
  }

  FAIL() << "Expected exception not thrown";
}

TEST(RPC_EXCEPTION_TEST, TCP_NOT_ACCEPTING)
{
  BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
  b.pool_size(1);

  ServiceBase::shp s(new test_service_one<test_service_one_impl>());
  b.register_and_init_service(s);
  b.start();

  TcpFrontEnd tfe(b,5555);
  TcpInvoker ti("127.0.0.1", 5555);

  tfe.start();

  ServiceClientFactory::scptr scf;

  try 
  {
    scf.reset(new ServiceClientFactory( "test service", ti,
                                        basic_protocol::SOURCE_RELAY, 
                                        basic_protocol::TARGET_MARSHALL));
    b.pause_requests();
    scf->subscribe_service<test_service_one_client>("test_service_one");
    FAIL() << "Exception should have been thrown";
  }
  catch(InvokerException ie)
  {
    boost::system::error_code * e = boost::get_error_info<rbl_invoker_error_code>(ie);
    
    EXPECT_EQ(e->value(), basic_protocol::NOT_ACCEPTING_REQUESTS);
    return;
  }
  catch(BackEndException b)
  {
    FAIL() << "should not throw a BackEndException";
  }
  catch(...)
  {
    FAIL() << "Wrong type of exception";
  }
  
  FAIL() << "Expected exception not thrown";
}

TEST(RPC_EXCEPTION_TEST, IN_PROCESS_NOT_ESTABLISHED)
{
/*
  {
    BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
    b.pool_size(1);

    ServiceBase::shp s(new test_service_one<test_service_one_impl>());
    b.register_and_init_service(s);
    b.start();
    InProcessInvoker ipi(b);  
    
    try 
    {
      ipi.client_data()->request().set_service_ordinal(0);
      ipi.client_data()->request().set_request_ordinal(1);

      basic_protocol::ListServicesRequest lreq;
      lreq.SerializeToString( ipi.clent_data()->request()->
      

      basic_protocol::ListServicesResponse lres;
    
        
    }
    catch(InvokerException ie)
    {
      boost::system::error_code * e = boost::get_error_info<rbl_invoker_error_code>(ie);
      
      EXPECT_EQ(e->value(), basic_protocol::REQUEST_CLIENT_NOT_ESTABLISHED);
    }
    catch(BackEndException b)
    {
      FAIL() << "should not throw a BackEndException";
    }
    catch(...)
    {
      FAIL() << "Wrong type of exception";
    }
  }
  
  {
    BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
    b.pool_size(1);

    ServiceBase::shp s(new test_service_one<test_service_one_impl>());
    b.register_and_init_service(s);
    b.start();
    InProcessInvoker ipi(b);  
     
    basic_protocol::HelloRequest hreq;
    basic_protocol::HelloResponse hres;

    hreq.set_source_type(basic_protocol::SOURCE_RELAY);
    hreq.set_expected_target(basic_protocol::TARGET_MARSHALL);
    hreq.set_node_name("test_client");
  
    ipi.client_data()->request().set_service_ordinal(0);
    ipi.client_data()->request().set_request_ordinal(0);

    basic_protocol::ListServicesRequest lreq;
    basic_protocol::ListServicesResponse lres;
      
  }
  */
}


#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
  ::testing::InitGoogleTest(&argc,argv);
 return RUN_ALL_TESTS();
}
#endif
