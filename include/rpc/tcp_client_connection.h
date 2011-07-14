#include <boost/asio.hpp>

namespace rubble { namespace rpc {

template<typename Derived>
class ClientConnection
{
public:
  ClientConnection(const std::string & client_name)
    : m_is_connected(false),
      m_name(client_name) {}
  void connect()
  {
    static_cast<Derived *>(this)->connect();
  };
  const boost::system::error_code & error() const { return m_ec; } 

  const bool connected() const { return m_is_connected; } 

  void connected(bool & connected) { m_is_connected = connected; }

  const boost::system::error_code & error_code() const { return m_ec; } 
protected:
  std::string m_name;
  bool m_is_connected;
  boost::system::error_code m_ec;
};

class TcpClientConnection : public ClientConnection<TcpClientConnection>
{
public:
  TcpClientConnection(  boost::asio::io_service & io_s , 
                        const std::string & name_in, 
                        const std::string & addr_in, 
                        const unsigned int port)
    : ClientConnection<TcpClientConnection>(name_in),
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
  void connect()
  {
    m_socket.connect(m_endpoint,m_ec);  
  }
private:
  const unsigned int m_port;
  boost::asio::ip::tcp::socket m_socket;
  boost::asio::ip::tcp::endpoint m_endpoint;
  boost::asio::ip::address m_ip_addr;
};

} }
