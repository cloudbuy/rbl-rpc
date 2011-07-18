#include <rpc/tcp_client_connection.h>
#include "rpc/client_rpc_common.h"


using rubble::rpc::TcpClientConnection;

int main()
{
  boost::asio::io_service io_s;
  TcpClientConnection con(io_s, rubble::rpc::basic_protocol::SOURCE_GENERATOR,rubble::rpc::basic_protocol::TARGET_MARSHALL, "test_client", "127.0.0.1", 5000);
  std::cout << con.error().value() << std::endl;
  con.connect();
  std::cout << con.error().value() << std::endl;
  rubble::rpc::ClientServiceFactory csf(con); 
}
