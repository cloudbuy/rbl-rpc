#ifndef RBL_RPC_CLIENT_BASE_H
#define RBL_RPC_CLIENT_BASE_H
namespace rubble {
namespace rpc {

  template<typename INVOKER>
  class ClientServiceBase
  {
  public:
    ClientServiceBase(INVOKER & m_invoker, boost::uint16_t method_count)
      : m_method_count(method_count)
    {
    }
  protected:
    common::OidContainer<common::Oid, boost::uint16_t>  m_service_method_map;
    boost::int16_t                                      m_method_count;
    INVOKER &                                           m_invoker;
  };

} }
#endif 
