#ifndef RBL_RPC_CLIENT_COOKIES_H
#define RBL_RPC_CLIENT_COOKIES_H
#include <rpc/backend/ClientData.h>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <map>


namespace rubble { namespace rpc {
  class ClientCookieBase
  {
  public:
    virtual ~ClientCookieBase() {}
  private:
  };
 
  class ClientCookie
  {
  public:
    typedef ClientCookie * ptr;  
  
    ClientCookie()
      : m_cookie_base(NULL),
        m_subscribed(false) {}

    ~ClientCookie() {if(m_cookie_base!=NULL) delete m_cookie_base; m_cookie_base=NULL; }

    bool is_subscribed()
    {
      return m_subscribed;
    }

    void subscribe()
    {
      m_subscribed = true;
    }

    template<typename T>
    void create_cookie(T ** cookie)
    {
      BOOST_ASSERT( (m_cookie_base == NULL));

      m_cookie_base = new T();
      *cookie = static_cast<T*>(m_cookie_base);
    }

    template<typename T>
    void retrieve_cookie(T ** cookie)
    {
      BOOST_ASSERT( (m_cookie_base != NULL));

      *cookie = static_cast<T*>(m_cookie_base);
    }

    void destroy_cookie()
    {
      if( m_cookie_base != NULL)
      {
        delete m_cookie_base;
        m_cookie_base=NULL;
      }
    }
  private:
    ClientCookieBase * m_cookie_base;
    bool m_subscribed;
  };
  
     
  class ClientServiceCookies : public boost::noncopyable
  {
  public:
    typedef std::map<ClientData *, ClientCookie> t_service_client_cookie_map;
    typedef std::vector<t_service_client_cookie_map *> t_service_cookie_vector;
    
    enum OP_ERROR_CODE {
      SUCCESS = 0,
      COOKIE_ABSENT
    };
   
    ~ ClientServiceCookies();
  
    void set_size(std::size_t sz);
    
    OP_ERROR_CODE delete_cookie(  common::ordinal_type service_ordinal, 
                                  ClientData * data_key);
    
    OP_ERROR_CODE cookie_count(common::ordinal_type service_ordinal, std::size_t & sz);
     
    void activate_service_with_ordinal(common::ordinal_type ordinal);
    
    
    void create_or_retrieve_cookie
      ( common::ordinal_type service_ordinal,
        ClientData * data_key,
        ClientCookie ** cookie );
    
    OP_ERROR_CODE contains_cookie ( common::ordinal_type service_ordinal, 
                                    ClientData * data_key) const; 
  private:
    t_service_cookie_vector m_service_cookie_vector;
  };


} }
#endif
