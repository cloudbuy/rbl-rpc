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
      : m_socket(io_s),
        m_flags(0),
        m_buffer_size(RBL_RPC_CONF_INITIAL_BUFFER_SIZE),
        m_buffer(new boost::uint8_t[RBL_RPC_CONF_INITIAL_BUFFER_SIZE]) {}

    tcp::socket & socket() { return m_socket; }
    
    boost::uint8_t * resize_buffer(std::size_t new_size)
    {
      m_buffer.reset(new boost::uint8_t[new_size]);
      m_buffer_size=new_size;
      return m_buffer.get();
    }
    std::size_t buffer_size() { return m_buffer_size; }
    
    boost::uint8_t * buffer() { return m_buffer.get(); } 
    boost::uint32_t & flags() { return m_flags; }
    basic_protocol::ClientRequest request() { return m_request; }
  private:
    tcp::socket m_socket;
    basic_protocol::ClientRequest m_request;
   
    std::size_t m_buffer_size;
    boost::scoped_array<boost::uint8_t> m_buffer;
    boost::uint32_t m_flags;
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
      m_connected_list.push_back(*ccp);
      boost::asio::async_read(  ccp->socket(), 
                                boost::asio::buffer(ccp->buffer(), 8),
                                boost::bind(&RpcOracle::handle_read_header, 
                                  this, ccp, boost::asio::placeholders::error));
    }

    void handle_read_header(  client_connection_ptr ccp, 
                              const boost::system::error_code & error)
    {
      if(!error)
      {
        boost::uint32_t msg_sz;
        google::protobuf::io::CodedInputStream cis(ccp->buffer(),8);
        
        cis.ReadLittleEndian32(&ccp->flags());
        cis.ReadLittleEndian32(&msg_sz);
        
        if(msg_sz < ccp->buffer_size())
          ccp->resize_buffer(msg_sz);
        
        boost::asio::async_read(  ccp->socket(), 
                                  boost::asio::buffer(ccp->buffer(), msg_sz),
                                  boost::bind(&RpcOracle::handle_read_message, 
                                  this, ccp, boost::asio::placeholders::bytes_transferred,
                                  boost::asio::placeholders::error));
      }
    }
    
    void handle_read_message( client_connection_ptr ccp,
                              std::size_t bytes_sent, 
                              const boost::system::error_code & error)
    {
      if(!error)
      {
        ccp->request().ParseFromArray( ccp->buffer(), bytes_sent);
      }
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
