#include "server_rpc_common.h"
#include <rpc/proto/BasicProtocol-server.rblrpc.h>
#include <iostream>

using rubble::rpc::basic_protocol::TARGET_RELAY;
using rubble::rpc::basic_protocol::TARGET_MARSHALL;

namespace rubble { namespace rpc {

  using namespace boost::asio;
  using namespace boost::asio::ip;
  using namespace rubble::rpc::basic_protocol;

  class ClientConnection
  {
  public:
    ClientConnection(io_service & io_s)
      : m_socket(io_s) {}
    tcp::socket & socket() { return m_socket; }
  private:
    tcp::socket m_socket;  
  };

  typedef boost::shared_ptr<ClientConnection> client_connection_ptr;
  
  class RpcOracle
  {
  public:
    RpcOracle(const DestinationConnectionType & con_type)
      : m_connection_type(con_type)
    {
      ServiceBase_shp basic_protocol(new BasicProtocol<>());
      m_services.RegisterService(basic_protocol);
    }
    void AddNewClient(client_connection_ptr ccp)
    {
      m_connected_list.push_back(ccp);
    }
    
  private:
    std::list<client_connection_ptr>  m_connected_list;
    ServiceOracle                     m_services;
    DestinationConnectionType         m_connection_type;
  };
  
  class ClientConnectionAcceptor
  {
  public:
    ClientConnectionAcceptor(RpcOracle & rpc_oracle, unsigned short port)
      : m_io_service(),
        m_port(port),
        m_endpoint(tcp::v4(), m_port),
        m_rpc_oracle(rpc_oracle),
        m_acceptor(m_io_service, m_endpoint)
    {
      client_connection_ptr ccp(new ClientConnection(m_io_service));
      m_acceptor.async_accept(  
        ccp->socket(),
        boost::bind(&ClientConnectionAcceptor::handle_accept,this, 
        ccp, placeholders::error));
    } 
    
    void handle_accept(client_connection_ptr ccp, const boost::system::error_code & error)
    {
      if(!error)
      {
        m_rpc_oracle.AddNewClient(ccp);
        client_connection_ptr new_ccp(new ClientConnection(m_io_service));
        m_acceptor.async_accept(  
          new_ccp->socket(),
          boost::bind(&ClientConnectionAcceptor::handle_accept,this, 
          ccp, placeholders::error));

      }
    }
    
    boost::asio::io_service & io_service() { return m_io_service; }
  private:
    boost::asio::io_service m_io_service;
    unsigned short m_port;
    tcp::endpoint m_endpoint;
    tcp::acceptor m_acceptor;
    RpcOracle & m_rpc_oracle;
  };

} }
