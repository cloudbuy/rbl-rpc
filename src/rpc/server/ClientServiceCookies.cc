#include <rpc/server/ClientServiceCookies.h>

namespace rubble { namespace rpc {
// ClientServiceCookies ///////////////////////////////////////////////////////

  #define OP_ERROR_CODE ClientServiceCookies::OP_ERROR_CODE
  // ~ClientServiceCookies ////////////////////////////////////////////////////
  ClientServiceCookies::~ClientServiceCookies()
  {
    BOOST_FOREACH(t_service_client_cookie_map * sm, m_service_cookie_vector)
    {
      if( sm != NULL)
        delete sm;
    }
  } 
  //-------------------------------------------------------------------------//
  
  // set_size /////////////////////////////////////////////////////////////////
  void ClientServiceCookies::set_size(std::size_t sz)
  {
    BOOST_ASSERT_MSG( !m_service_cookie_vector.size() > 0,
      "SIZE MAY ONLY BE SET ONCE");
          
    m_service_cookie_vector.resize(sz,NULL);
  }
 //-------------------------------------------------------------------------//

  // delete_cookie /////////////////////////////////////////////////////////// 
  OP_ERROR_CODE ClientServiceCookies::
  delete_cookie(  common::ordinal_type service_ordinal,
                  ClientData * data_key)
  {
    BOOST_ASSERT_MSG( !(service_ordinal > m_service_cookie_vector.size()-1),
      "SERVICE ORDINAL TO RETRIEVE COOKIE IS OUT OF RANGE, THIS SHOULD HAVE BEEN"
      " VERIFIED EARLIER");


      t_service_client_cookie_map * cm = m_service_cookie_vector[service_ordinal];
      
      BOOST_ASSERT_MSG(cm != NULL, 
        "THE ORIDNAL PROVIDED DOES NOT REFER TO A REGISTERED SERVICE, THIS SHOULD"
        " HAVE BEEN VERFIEID EARLIER");
 
      std::size_t r = cm->erase(data_key);
      if(r == 0)
      {
        return COOKIE_ABSENT;    
      }
      else 
      {
        return SUCCESS;
      }
  }
  //-------------------------------------------------------------------------//

  // cookie_count ///////////////////////////////////////////////////////////// 
  OP_ERROR_CODE ClientServiceCookies::
  cookie_count(common::ordinal_type service_ordinal, std::size_t & sz)
  {
    BOOST_ASSERT_MSG( !(service_ordinal > m_service_cookie_vector.size()-1),
      "SERVICE ORDINAL TO RETRIEVE COOKIE IS OUT OF RANGE, THIS SHOULD HAVE BEEN"
      " VERIFIED EARLIER");

    t_service_client_cookie_map * cm = m_service_cookie_vector[service_ordinal];
    
     BOOST_ASSERT_MSG((cm != NULL), 
        "THE ORIDNAL PROVIDED DOES NOT REFER TO A REGISTERED SERVICE, THIS SHOULD"
        " HAVE BEEN VERFIEID EARLIER");

    sz=cm->size();
    return SUCCESS;
  }
 //--------------------------------------------------------------------------//

  // activate_service_with_ordinal ////////////////////////////////////////////
  void ClientServiceCookies::
  activate_service_with_ordinal(common::ordinal_type ordinal)
  { 
    BOOST_ASSERT_MSG( m_service_cookie_vector.size() > ordinal ,
      "SERVICE ORDINAL IS OUT OF RANGE, THIS SHOULD HAVE BEEN"
      " VERIFIED EARLIER");
   
    BOOST_ASSERT_MSG( (m_service_cookie_vector[ordinal] == 0),
      "SERVICE ALLREADY ACTIVE");

    m_service_cookie_vector[ordinal] = 
      new t_service_client_cookie_map();

  }
  //-------------------------------------------------------------------------//

  // create_or_retrieve_cookie ////////////////////////////////////////////////
  void ClientServiceCookies::create_or_retrieve_cookie
    ( common::ordinal_type service_ordinal,
      ClientData * data_key,
      ClientCookie ** cookie )
  {
    BOOST_ASSERT_MSG( !(service_ordinal > m_service_cookie_vector.size()-1),
      "SERVICE ORDINAL TO RETRIEVE COOKIE IS OUT OF RANGE, THIS SHOULD HAVE BEEN"
      " VERIFIED EARLIER");

    t_service_client_cookie_map * cm = m_service_cookie_vector[service_ordinal];
    
    BOOST_ASSERT_MSG( (cm != NULL), 
        "THE ORIDNAL PROVIDED DOES NOT REFER TO A REGISTERED SERVICE, THIS SHOULD"
        " HAVE BEEN VERFIEID EARLIER");
  
     
    typedef t_service_client_cookie_map::value_type v_t;

    t_service_client_cookie_map::iterator it = cm->find(data_key);
    if(it == cm->end())
    {
      it = ( cm->insert( v_t(data_key,ClientCookie() ) ).first ) ;  
      *cookie = &it->second;
    }
    else
    {
      *cookie = &it->second;
    }    
  }
  //-------------------------------------------------------------------------//

  // contains_cookie //////////////////////////////////////////////////////////
  OP_ERROR_CODE ClientServiceCookies::
  contains_cookie(common::ordinal_type service_ordinal, ClientData * data_key) 
    const 
  {
    BOOST_ASSERT_MSG( ! (service_ordinal > m_service_cookie_vector.size()-1),
      "SERVICE ORDINAL TO RETRIEVE COOKIE IS OUT OF RANGE, THIS SHOULD HAVE BEEN"
      " VERIFIED EARLIER");
   
    t_service_client_cookie_map * cm = m_service_cookie_vector[service_ordinal];
    
    BOOST_ASSERT_MSG( (cm != NULL), 
      "THE ORIDNAL PROVIDED DOES NOT REFER TO A REGISTERED SERVICE, THIS SHOULD"
      " HAVE BEEN VERFIEID EARLIER");
 
    typedef t_service_client_cookie_map::value_type v_t;

    t_service_client_cookie_map::iterator it = cm->find(data_key);
    if(it == cm->end())
    {
      return COOKIE_ABSENT;    
    }
    else 
      return SUCCESS;
  }
  //-------------------------------------------------------------------------//
  #undef OP_ERROR_CODE
//---------------------------------------------------------------------------//
} }
