#include <rpc/server_rpc_common.h>
#include <rpc/TcpServer.h>

using namespace rubble::rpc;

int main()
{
  RpcOracle rpc_oracle(TARGET_MARSHALL);  
  ClientConnectionAcceptor acceptor(rpc_oracle, 5000 );
  acceptor.io_service().run();
}
