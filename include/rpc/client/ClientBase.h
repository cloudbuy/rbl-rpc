#ifndef RBL_RPC_CLIENT_BASE_H
#define RBL_RPC_CLIENT_BASE_H
#include <rpc/invoker/InvokerBase.h>

namespace rubble {
namespace rpc {

  class ClientServiceBase
  {
  public:
    ClientServiceBase(InvokerBase & invoker, boost::uint16_t method_count)
      : m_invoker(invoker),
        m_method_count(method_count)
    {
    }
  protected:
    common::OidContainer<common::Oid, boost::uint16_t>  m_service_method_map;
    boost::int16_t                                      m_method_count;
    InvokerBase &                                       m_invoker;
  };

} }
#endif 
