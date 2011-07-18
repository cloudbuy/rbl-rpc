#ifndef RBL_RPC_TCP_CLIENT_CONNECTION_H 
#define RBL_RPC_TCP_CLIENT_CONNECTION_H 

#include <boost/asio.hpp>
#include "rpc_common.h"
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

namespace rubble { namespace rpc {

class ClientConnection
{
public:
  ClientConnection( const std::string & client_name,
                    basic_protocol::SourceConnectionType & sct,
                    basic_protocol::DestinationConnectionType dct)
    : m_buffer_size(RBL_RPC_CONF_INITIAL_BUFFER_SIZE),
      m_buffer(new boost::uint8_t[RBL_RPC_CONF_INITIAL_BUFFER_SIZE]),
      m_is_connected(false),
      m_name(client_name),
      m_src_con_type(sct), 
      m_dst_con_type(dct) {}

  virtual void connect() = 0;
  virtual void rpc_call_dispatch( basic_protocol::ClientRequest & req, 
                                  basic_protocol::ClientResponse & res) = 0;
  void resize_buffer(std::size_t new_size)
  {
    m_buffer.reset(new boost::uint8_t[new_size]);
    m_buffer_size=new_size;
  }

  const boost::system::error_code & error() const { return m_ec; } 

  const bool connected() const { return m_is_connected; } 

  const boost::system::error_code & error_code() const { return m_ec; } 

  const std::string & name() const { return m_name; }
  
  const basic_protocol::SourceConnectionType & source_connection_type() const
    { return m_src_con_type; }
   const basic_protocol::DestinationConnectionType & destination_connection_type() const
    { return m_dst_con_type; }
 
protected:
  std::size_t m_buffer_size;
  boost::scoped_array<boost::uint8_t> m_buffer;

  std::string m_name;
  bool m_is_connected;
  boost::system::error_code m_ec;
  basic_protocol::SourceConnectionType m_src_con_type;
  basic_protocol::DestinationConnectionType m_dst_con_type;
};

class TcpClientConnection : public ClientConnection
{
public:
  TcpClientConnection(  boost::asio::io_service & io_s ,
                        basic_protocol::SourceConnectionType sct,
                        basic_protocol::DestinationConnectionType dct,
                        const std::string & name_in, 
                        const std::string & addr_in, 
                        const unsigned int port)
    : ClientConnection(name_in,sct,dct),
      m_socket(io_s),
      m_port(port) 
  {
    boost::asio::ip::address m_ip_addr =
      boost::asio::ip::address::from_string(addr_in,m_ec);
    if(!m_ec)
    {
      m_endpoint.address(m_ip_addr);
      m_endpoint.port(m_port);
    }
  }
  virtual void connect()
  {
    m_socket.connect(m_endpoint,m_ec);
    if(!m_ec)
      m_is_connected = true;
  }

  virtual void rpc_call_dispatch( basic_protocol::ClientRequest & req, 
                                  basic_protocol::ClientResponse & res)
  {
    std::cout << "CD1" << std::endl;
    boost::uint32_t flag_return;
    boost::uint32_t msg_size_return;

    boost::uint32_t msg_size = 8 + req.ByteSize();
    if(msg_size > m_buffer_size)
      resize_buffer(msg_size);
      
    google::protobuf::io::ArrayOutputStream aos(m_buffer.get(),m_buffer_size);
    google::protobuf::io::CodedOutputStream cos(&aos);
    
    cos.WriteLittleEndian32(0); // FOR RPC FLAGS -- UNUSED ATM
    cos.WriteLittleEndian32(msg_size);
    req.SerializeToCodedStream(&cos);
    std::size_t transfered_count = 
      boost::asio::write(m_socket, 
        boost::asio::buffer(m_buffer.get(), msg_size),m_ec);

    if(!m_ec)
    {
      boost::asio::read(m_socket, boost::asio::buffer(m_buffer.get(), 8),m_ec);
      if(!m_ec)
      {
        google::protobuf::io::CodedInputStream cis(m_buffer.get(),m_buffer_size);
        cis.ReadLittleEndian32(&flag_return);
        cis.ReadLittleEndian32(&msg_size_return);
        
        if(msg_size_return < m_buffer_size)
          resize_buffer(msg_size_return);
        
        transfered_count = boost::asio::read(m_socket, 
          boost::asio::buffer(m_buffer.get(), msg_size_return),m_ec);
      }
    }
  }
private: 
  const unsigned int m_port;
  boost::asio::ip::tcp::socket m_socket;
  boost::asio::ip::tcp::endpoint m_endpoint;
  boost::asio::ip::address m_ip_addr;
};

} }
#endif 
