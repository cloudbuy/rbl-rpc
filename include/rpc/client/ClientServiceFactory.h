#include <rpc/backend/BackEndBase.h>
#include <rpc/proto/BasicProtocol-client.rblrpc.h>

namespace rubble {
namespace rpc {

template<typename INVOKER>
class ServiceClientFactory
{
public:
  ServiceClientFactory(INVOKER & invoker)
    : m_invoker(& invoker)
  {
    if( ! m_invoker.is_useable() ) 
      throw BackEndException(); 
  }
private:
  INVOKER & m_invoker;
};

} } 
