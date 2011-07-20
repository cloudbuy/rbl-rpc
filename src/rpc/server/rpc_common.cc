#include <rpc/server/rpc_common.h>

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
  OP_ERROR_CODE ClientServiceCookies::set_size(std::size_t sz)
  {
    if( m_service_cookie_vector.size() > 0)
      return ALLREADY_SIZED;
    else
    {
      m_service_cookie_vector.resize(sz,NULL);
      return SUCCESS;
    }
  }
 //-------------------------------------------------------------------------//

  // delete_cookie_in_service ///////////////////////////////////////////////// 
  OP_ERROR_CODE ClientServiceCookies::
  delete_cookie_in_service( common::ordinal_type service_ordinal,
                            ClientData * data_key)
  {
      if(service_ordinal > m_service_cookie_vector.size()-1)
        return ORDINAL_OUT_OF_RANGE;

      t_service_client_cookie_map * cm = m_service_cookie_vector[service_ordinal];
      
      if(cm == NULL) 
        return UNUSED_SERVICE_ORDINAL;
 
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
    if(service_ordinal > m_service_cookie_vector.size()-1)
      return ORDINAL_OUT_OF_RANGE;

    t_service_client_cookie_map * cm = m_service_cookie_vector[service_ordinal];
    
    if(cm == NULL) 
      return UNUSED_SERVICE_ORDINAL;

    sz=cm->size();
    return SUCCESS;
  }
 //--------------------------------------------------------------------------//

  // activate_service_with_ordinal ////////////////////////////////////////////
  OP_ERROR_CODE ClientServiceCookies::
  activate_service_with_ordinal(common::ordinal_type ordinal)
  { 
    if( ordinal > m_service_cookie_vector.size()-1)
      return ORDINAL_OUT_OF_RANGE;
    
    if( m_service_cookie_vector[ordinal] != 0)  
      return ORDINAL_ALLREADY_ACTIVATED;

    m_service_cookie_vector[ordinal] = 
      new t_service_client_cookie_map();

    return SUCCESS;
  }
  //-------------------------------------------------------------------------//

  // create_or_retrieve_cookie ////////////////////////////////////////////////
  OP_ERROR_CODE ClientServiceCookies::create_or_retrieve_cookie
    ( common::ordinal_type service_ordinal,
      ClientData * data_key,
      ClientCookiePtrContainer ** cookie )
  {
    if(service_ordinal > m_service_cookie_vector.size()-1)
      return ORDINAL_OUT_OF_RANGE;

    t_service_client_cookie_map * cm = m_service_cookie_vector[service_ordinal];
    
    if(cm == NULL) 
      return UNUSED_SERVICE_ORDINAL;
    
    typedef t_service_client_cookie_map::value_type v_t;

    t_service_client_cookie_map::iterator it = cm->find(data_key);
    if(it == cm->end())
    {
      it = ( cm->insert( v_t(data_key,ClientCookiePtrContainer() ) ).first ) ;  
      *cookie = &it->second;
      return SUCCESS;
    }
    else
    {
      *cookie = &it->second;
      return SUCCESS;
    }    
  }
  //-------------------------------------------------------------------------//

  // contains_cookie //////////////////////////////////////////////////////////
  OP_ERROR_CODE ClientServiceCookies::
  contains_cookie(common::ordinal_type service_ordinal, ClientData * data_key) 
  {
    if(service_ordinal > m_service_cookie_vector.size()-1)
        return ORDINAL_OUT_OF_RANGE;

    t_service_client_cookie_map * cm = m_service_cookie_vector[service_ordinal];
    
    if(cm == NULL) 
      return UNUSED_SERVICE_ORDINAL;
    
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
