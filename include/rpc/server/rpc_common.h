#ifndef RBL_RPC_SERVER_RPC_COMMON_H
#define RBL_RPC_SERVER_RPC_COMMON_H
#include <rpc/common/rpc_common.h>
#include <boost/foreach.hpp>

namespace rubble { namespace rpc {

  class ServiceBase
  {
  public:
    virtual bool Init() =0;
    virtual bool TearDown()=0;
    virtual bool Dispatch( void * client_cookie, basic_protocol::ClientRequest & cr)=0;
    virtual const char * name() = 0;
    virtual ~ServiceBase(){};
    
    common::ordinal_type ordinal() { return m_ordinal; }
    void set_ordinal(common::ordinal_type ordinal) { m_ordinal = ordinal; }
  private:
    common::ordinal_type m_ordinal;
  };
} } 

#include <rpc/proto/BasicProtocol-server.rblrpc.h>

namespace rubble { namespace rpc {
/*
  class BasicProtocol
  {
  public:
    bool Init() {}
    bool TearDown() {}
    bool Hello(HelloRequest,HelloResponse){};
    bool ListServices(ListServicesRequest,ListServicesResponse){};
  private:
  };
*/
  class ClientData
  {
  public:
    ClientData() 
      : m_flags(0),
        m_buffer_size(RBL_RPC_CONF_INITIAL_BUFFER_SIZE),
        m_buffer(new boost::uint8_t[RBL_RPC_CONF_INITIAL_BUFFER_SIZE]) {}

    boost::uint8_t * resize_buffer(std::size_t new_size)
    {
      m_buffer.reset(new boost::uint8_t[new_size]);
      m_buffer_size=new_size;
      return m_buffer.get();
    }
    
    boost::uint8_t * buffer() { return m_buffer.get(); } 
    boost::uint32_t & flags() { return m_flags; }
    basic_protocol::ClientRequest request() { return m_request; }

    std::size_t buffer_size() { return m_buffer_size; }
  private:
    basic_protocol::ClientRequest m_request;
    std::string name;
   
    std::size_t m_buffer_size;
    boost::scoped_array<boost::uint8_t> m_buffer;
    boost::uint32_t m_flags;
  };

  typedef boost::shared_ptr<ServiceBase> ServiceBase_shp;

  class ClientCookieBase
  {   
  public:
    virtual ~ClientCookieBase() {}
  };
    
  class ClientCookiePtrContainer
  {
  public:
    ClientCookiePtrContainer()
      : m_cookie_base(NULL) {}
    ~ClientCookiePtrContainer() {if(m_cookie_base!=NULL) delete m_cookie_base; }

    void ptr(ClientCookieBase * cookie_in) { m_cookie_base = cookie_in; }
    ClientCookieBase * ptr() { return m_cookie_base; }
    
  private:
    ClientCookieBase * m_cookie_base;
  };

  class ClientServiceCookies
  {
  public:
    typedef std::map<ClientData *, ClientCookiePtrContainer> t_service_client_cookie_map;
    typedef std::vector<t_service_client_cookie_map *> t_service_cookie_vector;
    
    enum OP_ERROR_CODE {
      SUCCESS = 0,
      ALLREADY_SIZED,
      ORDINAL_OUT_OF_RANGE,
      ORDINAL_ALLREADY_ACTIVATED,
      UNUSED_SERVICE_ORDINAL,
      COOKIE_ABSENT
    };
    
    ~ ClientServiceCookies();
  
    OP_ERROR_CODE set_size(std::size_t sz);
    
    OP_ERROR_CODE delete_cookie_in_service(  common::ordinal_type service_ordinal,
                                              ClientData * data_key);
    
    OP_ERROR_CODE cookie_count(common::ordinal_type service_ordinal, std::size_t & sz);
     
    OP_ERROR_CODE activate_service_with_ordinal(common::ordinal_type ordinal);
    
    
    OP_ERROR_CODE create_or_retrieve_cookie
      ( common::ordinal_type service_ordinal,
        ClientData * data_key,
        ClientCookiePtrContainer ** cookie );
    
    OP_ERROR_CODE contains_cookie ( common::ordinal_type service_ordinal, 
                                    ClientData * data_key); 
  private:
    t_service_cookie_vector m_service_cookie_vector;
  };

  class ServiceOracle 
  {
  public:
    ServiceOracle()
      : m_services(),
        m_is_sealed(false),
        m_service_count(0) 
    {
    }
  
    bool RegisterService(ServiceBase_shp service)
    {
      if(!m_is_sealed)
      {
        common::OP_RESPONSE ret = m_services.SetEntry(
          common::Oid(service->name(),m_service_count),service);
        if(ret == common::OP_NO_ERROR)
        {
          m_service_count++; 
          return true; 
        }
        else 
          throw "Service ordinal or name, allready exists;";
      }
        else throw "Service Oracle is final/sealed, cannot add more entries";
    }
    void seal()
    {
      
      m_is_sealed = true;
    }
  private:
    common::OidContainer<common::Oid, ServiceBase_shp> m_services;
    boost::uint8_t m_service_count;
    bool m_is_sealed;
  };
  
} }

#endif 
