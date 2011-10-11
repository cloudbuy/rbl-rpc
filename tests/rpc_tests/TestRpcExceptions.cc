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

// Note:
//
// the double hello issue is tested in the ServiceClientFactory Constructor

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
    
    EXPECT_EQ(e->value(), basic_protocol::REQUEST_BACKEND_NOT_ACCEPTING_REQUESTS);
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
  tfe.start();

  TcpInvoker ti("127.0.0.1", 5555);


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
    
    EXPECT_EQ(e->value(), basic_protocol::REQUEST_BACKEND_NOT_ACCEPTING_REQUESTS);
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

// bypassing encapsulation for testing purposes
class BPC : public basic_protocol::basic_protocol_client
{
public:
  BPC(InvokerBase & invoker)
    :  basic_protocol::basic_protocol_client(invoker) {}
  
  void remap_ordinals(t_service_method_map & m_in)
  {
    basic_protocol::basic_protocol_client::remap_ordinals(m_in);
  }
  void set_service_ordinal(boost::uint16_t ordinal)
  {
    basic_protocol::basic_protocol_client::set_service_ordinal(ordinal);
  }
};

BasicProtocolMethodMap m_method_map;

TEST(RPC_EXCEPTION_TEST, IN_PROCESS_NOT_ESTABLISHED)
{
    BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
    b.pool_size(1);

    ServiceBase::shp s(new test_service_one<test_service_one_impl>());
    b.register_and_init_service(s);
    b.start();
    InProcessInvoker ipi(b);  
    
    BPC bpc(ipi);

    bpc.set_service_ordinal(0);
    bpc.remap_ordinals(m_method_map);

    try 
    {
      basic_protocol::ListServicesRequest lreq;
      basic_protocol::ListServicesResponse lres;
    
      bpc.list_services(lreq,lres);
    }
    catch(InvokerException ie)
    {
      boost::system::error_code * e = boost::get_error_info<rbl_invoker_error_code>(ie);
      
      EXPECT_EQ(e->value(), basic_protocol::REQUEST_CLIENT_NOT_ESTABLISHED);
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


TEST(RPC_EXCEPTION_TEST, TCP_PROCESS_NOT_ESTABLISHED)
{
    BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
    b.pool_size(1);

    ServiceBase::shp s(new test_service_one<test_service_one_impl>());
    b.register_and_init_service(s);
    b.start();
    
    TcpFrontEnd tfe(b,5555);
    tfe.start();

    TcpInvoker ti("127.0.0.1", 5555);

    BPC bpc(ti);

    bpc.set_service_ordinal(0);
    bpc.remap_ordinals(m_method_map);

    try 
    {
      basic_protocol::ListServicesRequest lreq;
      basic_protocol::ListServicesResponse lres;
    
      bpc.list_services(lreq,lres);
    }
    catch(InvokerException ie)
    {
      boost::system::error_code * e = boost::get_error_info<rbl_invoker_error_code>(ie);
      
      EXPECT_EQ(e->value(), basic_protocol::REQUEST_CLIENT_NOT_ESTABLISHED);
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

TEST(RPC_EXCEPTION_TEST, IN_PROCESS_NO_SERVICE_WITH_ORDINAL)
{
    BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
    b.pool_size(1);

    ServiceBase::shp s(new test_service_one<test_service_one_impl>());
    b.register_and_init_service(s);
    b.start();
    InProcessInvoker ipi(b);  
    
    BPC bpc(ipi);

    bpc.set_service_ordinal(100);
    bpc.remap_ordinals(m_method_map);

    try 
    {
      basic_protocol::ListServicesRequest lreq;
      basic_protocol::ListServicesResponse lres;
    
      bpc.list_services(lreq,lres);
    }
    catch(InvokerException ie)
    {
      boost::system::error_code * e = boost::get_error_info<rbl_invoker_error_code>(ie);
      
      EXPECT_EQ(e->value(), basic_protocol::REQUEST_NO_SERVICE_WITH_ORDINAL);
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

TEST(RPC_EXCEPTION_TEST, TCP_PROCESS_NO_SERVICE_WITH_ORDINAL)
{
    BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
    b.pool_size(1);

    ServiceBase::shp s(new test_service_one<test_service_one_impl>());
    b.register_and_init_service(s);
    b.start();
    
    TcpFrontEnd tfe(b,5555);
    tfe.start();

    TcpInvoker ti("127.0.0.1", 5555);

    BPC bpc(ti);

    bpc.set_service_ordinal(100);
    bpc.remap_ordinals(m_method_map);

    try 
    {
      basic_protocol::ListServicesRequest lreq;
      basic_protocol::ListServicesResponse lres;
    
      bpc.list_services(lreq,lres);
    }
    catch(InvokerException ie)
    {
      boost::system::error_code * e = boost::get_error_info<rbl_invoker_error_code>(ie);
      
      EXPECT_EQ(e->value(), basic_protocol::REQUEST_NO_SERVICE_WITH_ORDINAL);
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

TEST(RPC_EXCEPTION_TEST, IN_PROCESS_NO_REQUEST_WITH_ORDINAL)
{
    BasicProtocolMethodMap m_method_map;

    *m_method_map[1]=100;

    BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
    b.pool_size(1);

    ServiceBase::shp s(new test_service_one<test_service_one_impl>());
    b.register_and_init_service(s);
    b.start();
    InProcessInvoker ipi(b);  
    
    BPC bpc(ipi);

    bpc.set_service_ordinal(0);
    bpc.remap_ordinals(m_method_map);

    try 
    {
      basic_protocol::ListServicesRequest lreq;
      basic_protocol::ListServicesResponse lres;
    
      bpc.list_services(lreq,lres);
    }
    catch(InvokerException ie)
    {
      boost::system::error_code * e = boost::get_error_info<rbl_invoker_error_code>(ie);
      
      EXPECT_EQ(e->value(), basic_protocol::REQUEST_NO_FUNCTION_WITH_ORDINAL);
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

TEST(RPC_EXCEPTION_TEST, TCP_PROCESS_NO_REQUEST_WITH_ORDINAL)
{
    BasicProtocolMethodMap m_method_map;

    *m_method_map[1]=100;

    BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
    b.pool_size(1);

    ServiceBase::shp s(new test_service_one<test_service_one_impl>());
    b.register_and_init_service(s);
    b.start();
    
    TcpFrontEnd tfe(b,5555);
    tfe.start();

    TcpInvoker ti("127.0.0.1", 5555);

    BPC bpc(ti);

    bpc.set_service_ordinal(0);
    bpc.remap_ordinals(m_method_map);

    try 
    {
      basic_protocol::ListServicesRequest lreq;
      basic_protocol::ListServicesResponse lres;
    
      bpc.list_services(lreq,lres);
    }
    catch(InvokerException ie)
    {
      boost::system::error_code * e = boost::get_error_info<rbl_invoker_error_code>(ie);
      
      EXPECT_EQ(e->value(), basic_protocol::REQUEST_NO_FUNCTION_WITH_ORDINAL);
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

TEST(RPC_EXCEPTION_TEST, IN_PROCESS_NOT_SUBSCRIBED)
{
    BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
    b.pool_size(1);

    ServiceBase::shp s(new test_service_one<test_service_one_impl>());
    b.register_and_init_service(s);
    b.start();
    InProcessInvoker ipi(b);  
    
    BPC bpc(ipi);

    // HACK!!!! forcing a server (1) and request(0) ordinal
    // i am not really sending a hello message... i am sending
    // a call to dummy rpc which should be unsubscribed
    bpc.set_service_ordinal(1);
    bpc.remap_ordinals(m_method_map);

    try 
    {
      basic_protocol::HelloRequest hreq;
      basic_protocol::HelloResponse hres;

      bpc.hello(hreq, hres); 
    }
    catch(InvokerException ie)
    {
      boost::system::error_code * e = boost::get_error_info<rbl_invoker_error_code>(ie);
      
      EXPECT_EQ(e->value(), basic_protocol::REQUEST_NOT_SUBSCRIBED);
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

TEST(RPC_EXCEPTION_TEST, TCP_PROCESS_NOT_SUBSCRIBED)
{
    BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
    b.pool_size(1);

    ServiceBase::shp s(new test_service_one<test_service_one_impl>());
    b.register_and_init_service(s);
    b.start();

    TcpFrontEnd tfe(b,5555);
    tfe.start();

    TcpInvoker ti("127.0.0.1", 5555);

    BPC bpc(ti);

    // HACK!!!! forcing a server (1) and request(0) ordinal
    // i am not really sending a hello message... i am sending
    // a call to dummy rpc which should be unsubscribed
    bpc.set_service_ordinal(1);
    bpc.remap_ordinals(m_method_map);

    try 
    {
      basic_protocol::HelloRequest hreq;
      basic_protocol::HelloResponse hres;

      bpc.hello(hreq, hres); 
    }
    catch(InvokerException ie)
    {
      boost::system::error_code * e = boost::get_error_info<rbl_invoker_error_code>(ie);
      
      EXPECT_EQ(e->value(), basic_protocol::REQUEST_NOT_SUBSCRIBED);
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

class SerErrInProcessInvoker : public InProcessInvoker
{
public:
  SerErrInProcessInvoker(BackEnd & b_in)
    : InProcessInvoker(b_in) {}
   
  bool invoker()
  {
    request().set_request_string("trash");
    return m_backend.invoke(*this);
  } 
};

class SerErrTcpInvoker : public TcpInvoker
{
public:
  SerErrTcpInvoker(const std::string & str, short port)
    : TcpInvoker(str,port) {} 
  
  bool invoke()
  {
      boost::uint32_t flag_return;
      boost::uint32_t msg_size_return=0;

      boost::uint32_t msg_size = 8 + request().ByteSize();
      
      if(msg_size > m_buffer.size() )
        m_buffer.resize(msg_size);
      
      google::protobuf::io::ArrayOutputStream aos(m_buffer.get(),m_buffer.size());
      google::protobuf::io::CodedOutputStream cos(&aos);
      
      cos.WriteLittleEndian32(0); // FOR RPC FLAGS -- UNUSED ATM
      cos.WriteLittleEndian32(msg_size);
      request().set_request_string("trash");
      request().SerializeToCodedStream(&cos);

      std::size_t transfered_count = 
        boost::asio::write(m_socket, 
          boost::asio::buffer(m_buffer.get(), msg_size),m_error_code);

      if(!m_error_code)
      {
        boost::asio::read(m_socket, boost::asio::buffer(m_buffer.get(), 8),m_error_code);
        if(!m_error_code)
        {
          google::protobuf::io::CodedInputStream cis(m_buffer.get(),m_buffer.size());
          
          cis.ReadLittleEndian32( & flag_return);
          cis.ReadLittleEndian32( & msg_size_return);
          
          if(msg_size_return > m_buffer.size())
            m_buffer.resize(msg_size_return);
          
          transfered_count = boost::asio::read(m_socket, 
            boost::asio::buffer(m_buffer.get(), msg_size_return-8),m_error_code);
          if(!m_error_code)
          {
            google::protobuf::io::CodedInputStream cis2(m_buffer.get(),m_buffer.size());

            response().ParseFromCodedStream(&cis2);
             
          }
          else
            std::cout << "error" << std::endl;
        }
        else
          std::cout << "error" << std::endl;
      }
      else
        std::cout << "error" << std::endl;
      return false;
  }

};

TEST(RPC_EXCEPTION_TEST, IN_PROCESS_REMOTE_SERIALIZATION_ERROR)
{
    BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
    b.pool_size(1);

    ServiceBase::shp s(new test_service_one<test_service_one_impl>());
    b.register_and_init_service(s);
    b.start();
    SerErrInProcessInvoker ipi(b);  
    
    BPC bpc(ipi);

    bpc.set_service_ordinal(0);
    bpc.remap_ordinals(m_method_map);

    try 
    {
      basic_protocol::HelloRequest hreq;
      basic_protocol::HelloResponse hres;

      bpc.hello(hreq, hres); 
    }
    catch(InvokerException ie)
    {
      boost::system::error_code * e = boost::get_error_info<rbl_invoker_error_code>(ie);
      
      EXPECT_EQ(e->value(), basic_protocol::REQUEST_STRING_PARSE_ERROR);
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

TEST(RPC_EXCEPTION_TEST, TCP_PROCESS_REMOTE_SERIALIZATION_ERROR)
{
    BackEnd b(basic_protocol::SOURCE_RELAY , basic_protocol::TARGET_MARSHALL);
    b.pool_size(1);

    ServiceBase::shp s(new test_service_one<test_service_one_impl>());
    b.register_and_init_service(s);
    b.start();

    TcpFrontEnd tfe(b,5555);
    tfe.start();
    SerErrTcpInvoker ti("127.0.0.1", 5555);

    BPC bpc(ti);

    bpc.set_service_ordinal(0);
    bpc.remap_ordinals(m_method_map);

    try 
    {
      basic_protocol::HelloRequest hreq;
      basic_protocol::HelloResponse hres;

      bpc.hello(hreq, hres); 
    }
    catch(InvokerException ie)
    {
      boost::system::error_code * e = boost::get_error_info<rbl_invoker_error_code>(ie);
      
      EXPECT_EQ(e->value(), basic_protocol::REQUEST_STRING_PARSE_ERROR);
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


#ifdef ISOLATED_GTEST_COMPILE
int main(int argc,char ** argv)
{
  ::testing::InitGoogleTest(&argc,argv);
 return RUN_ALL_TESTS();
}
#endif
