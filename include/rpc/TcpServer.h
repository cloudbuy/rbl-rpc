#include "server_rpc_common.h"
#include <rpc/proto/BasicProtocol-server.rblrpc.h>
#include <boost/intrusive/list.hpp>
#include <iostream>
#include <boost/enable_shared_from_this.hpp>

using rubble::rpc::basic_protocol::TARGET_RELAY;
using rubble::rpc::basic_protocol::TARGET_MARSHALL;
namespace rubble { namespace rpc {

  using namespace boost::asio;
  using namespace boost::asio::ip;
  using namespace rubble::rpc::basic_protocol;

  class ClientConnection : public boost::intrusive::list_base_hook<>
  {
  public:
    ClientConnection(io_service & io_s)
      : m_socket(io_s) {}
 
    tcp::socket & socket() { return m_socket; }
    ClientData & data() { return m_data; }
  private:
    tcp::socket m_socket;
    ClientData m_data;
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
      std::cout << " added new client" << std::endl;

      m_connected_list.push_back(*ccp);
      boost::asio::async_read(  ccp->socket(), 
                                boost::asio::buffer(ccp->data().buffer(), 8),
                                boost::bind(&RpcOracle::handle_read_header,
                                  this, ccp, 
                                  boost::asio::placeholders::bytes_transferred,
                                  boost::asio::placeholders::error));
    }

    void handle_read_header(  client_connection_ptr ccp, 
                              std::size_t bytes_transfered,
                              const boost::system::error_code & error)
    {
      if(!error)
      {
        std::cout << " read header" << std::endl;        
        boost::uint32_t msg_sz;
        google::protobuf::io::CodedInputStream cis(ccp->data().buffer(),8);
        
        cis.ReadLittleEndian32(&ccp->data().flags());
        cis.ReadLittleEndian32(&msg_sz);
       
        std::cout << ccp->data().flags() << "-" << msg_sz << "-"<<  bytes_transfered << std::endl;
 
        if(msg_sz < ccp->data().buffer_size())
          ccp->data().resize_buffer(msg_sz);


        boost::asio::async_read(  ccp->socket(), 
                                  boost::asio::buffer(ccp->data().buffer(), ( msg_sz -8)),
                                  boost::bind(&RpcOracle::handle_read_message, 
                                    this, ccp, 
                                    boost::asio::placeholders::bytes_transferred,
                                    boost::asio::placeholders::error));
      }
      else
        std::cout << "error" << std::endl;
    }
    
    void handle_read_message( client_connection_ptr ccp,
                              std::size_t bytes_sent, 
                              const boost::system::error_code & error)
    {
      if(!error)
      {
        std::cout << "handle read message" << std::endl;
        ccp->data().request().ParseFromArray( ccp->data().buffer(), bytes_sent);
        std::cout << "bytes sent" << std::endl;
      }
      else
        std::cout << "error" << std::endl;
    }
    
    void handle_write_response( client_connection_ptr ccp, 
                                const boost::system::error_code & error)
    {
      if(!error)
      {
      }
    }

  private:
    boost::intrusive::list<ClientConnection>    m_connected_list;
    ServiceOracle                               m_services;
    DestinationConnectionType                   m_connection_type;
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
      std::cout << "accept" << std::endl;
      client_connection_ptr ccp(new ClientConnection(m_io_service));
      m_acceptor.async_accept(  
        ccp->socket(),
        boost::bind(&ClientConnectionAcceptor::handle_accept,this, 
        ccp, placeholders::error));
    } 
    
    void handle_accept(client_connection_ptr ccp, const boost::system::error_code & error)
    {
      std::cout << "handle_accept" << std::endl;
      if(!error)
      {
        m_rpc_oracle.AddNewClient(ccp);
        client_connection_ptr new_ccp(new ClientConnection(m_io_service));
        m_acceptor.async_accept(  
          new_ccp->socket(),
          boost::bind(&ClientConnectionAcceptor::handle_accept,this, 
          ccp, placeholders::error));

      }
      else
        std::cout << "error" << std::endl;
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
