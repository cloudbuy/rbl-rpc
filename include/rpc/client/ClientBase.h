#ifndef RBL_RPC_CLIENT_BASE_H
#define RBL_RPC_CLIENT_BASE_H
#include <rpc/invoker/InvokerBase.h>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace rubble {
namespace rpc {
  class ServiceClientFactory;
      
  class ClientServiceBase
  {
  public:
    typedef boost::scoped_ptr<ClientServiceBase> scptr;
    typedef boost::shared_ptr<ClientServiceBase> shptr;

    typedef common::OidContainer<common::Oid, boost::uint16_t> t_service_method_map;

    ClientServiceBase(InvokerBase & invoker, boost::uint16_t method_count)
      : m_invoker(invoker),
        m_method_count(method_count)
    {
    }

  protected:
    friend class ServiceClientFactory;

    void remap_ordinals(t_service_method_map & m_in)
    {
      BOOST_ASSERT_MSG( m_in.size() == m_service_method_map.size(),
        "the remap operation is an exact 1 - 1 alteration, "
         "size of both maps should be equal.");
       
      for(int i = 0; i < m_in.size(); ++i)
      {
        const t_service_method_map::entry_type * e_in
          = m_in.EntryAtordinal(i);
      
        t_service_method_map::entry_type * e
          = m_service_method_map.EntryAtordinal(i);

        BOOST_ASSERT(e != NULL);
        BOOST_ASSERT(e_in != NULL);
        BOOST_ASSERT(e_in->name() == e->name() ); 
        
        e->set_entry(e_in->entry()); 
      } 
    }
    boost::uint16_t service_ordinal()
    {
      return m_service_ordinal;
    }
    void set_service_ordinal(boost::uint16_t ordinal)
    {
      m_service_ordinal = ordinal; 
    }
    
    boost::uint16_t         m_service_ordinal;
    t_service_method_map    m_service_method_map;
    boost::int16_t          m_method_count;
    InvokerBase &           m_invoker;
  };

} }
#endif 
