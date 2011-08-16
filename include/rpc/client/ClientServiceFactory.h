#include <rpc/backend/BackEndBase.h>
#include <rpc/invoker/InvokerBase.h>
#include <rpc/proto/BasicProtocol-client.rblrpc.h>

namespace rubble {
namespace rpc {


class ServiceClientFactory
{
public:
  ServiceClientFactory(InvokerBase & invoker)
    : m_invoker(invoker)
  {
    if( ! m_invoker.is_useable() ) 
      throw BackEndException(); 
  }
private:
  InvokerBase & m_invoker;
};

} } 
