#ifndef RBL_RPC_TCP_FRONT_END_H
#define RBL_RPC_TCP_FRONT_END_H
#include <rpc/backend/BackEndBase.h>
#include <rpc/backend/ClientData.h>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <set>

namespace rubble {
namespace rpc {

typedef boost::shared_ptr<boost::asio::ip::tcp::socket> SharedSocket;

class FrontEndException
{
};

struct TcpConnection
{
  typedef boost::shared_ptr<TcpConnection>  shptr;

  TcpConnection(BackEnd & b, SharedSocket s_in)
    : socket(s_in),
      backend(b)
  {
    
  }

  void handle_read_header(  std::size_t bytes_sent, const boost::system::error_code & error)
  {
    if(!error)
    {
      boost::uint32_t msg_sz;
      google::protobuf::io::CodedInputStream cis(buffer.get(),8);
          
      cis.ReadLittleEndian32( &cd.flags());
      cis.ReadLittleEndian32(&msg_sz);

      if(msg_sz < buffer.size())
            buffer.resize(msg_sz);

      boost::asio::async_read(  *socket.get(), 
                                    boost::asio::buffer( buffer.get(), ( msg_sz -8)),
                                    boost::bind(&TcpConnection::handle_read_body, 
                                      this, 
                                      boost::asio::placeholders::bytes_transferred,
                                      boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "error in handle_read_header" << std::endl;
    }
  }

  void handle_read_body(   std::size_t bytes_sent,
                          const boost::system::error_code & error)
  {
    if(!error)
    {
      cd.request().ParseFromArray( buffer.get(), bytes_sent);
    }
    else
    {
      std::cout <<"error in handle_ready_body" << std::endl;
    }
  }

  SharedSocket                  socket;
  ClientData                    cd;
  Buffer                        buffer;
  BackEnd &                     backend;
};

class TcpFrontEnd
{
public:
  TcpFrontEnd(BackEnd & b_in, short port)
    : m_io_service(),
      m_backend(b_in),
      m_acceptor( m_io_service, 
                  boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port))
  {
  }

  void start()
  {
    if( ! m_backend.is_useable() )   
      throw FrontEndException();
  }

  void start_accept()
  {
    SharedSocket socket(new boost::asio::ip::tcp::socket(m_io_service) );

    m_acceptor.async_accept( *socket.get(), 
      boost::bind(&TcpFrontEnd::handle_accept, this, 
        socket,boost::asio::placeholders::error));
  }
  


private:
    void handle_accept(SharedSocket socket, const boost::system::error_code & error)
  {
    if(!error)
    {
      TcpConnection::shptr connection(new TcpConnection( m_backend, socket));
       
      m_connections.insert(connection);

      boost::asio::async_read(  *connection->socket.get(),
                                  boost::asio::buffer ( &connection->buffer, 8 ),
                                  boost::bind ( &TcpConnection::handle_read_header, connection,
                                    boost::asio::placeholders::bytes_transferred,
                                    boost::asio::placeholders::error ) );
    }
    else
    {
      std::cout << "error in handle_accept" << std::endl;
    }
    start_accept();
  }

  

  BackEnd &                                   m_backend;
  boost::asio::io_service                     m_io_service;
  boost::asio::ip::tcp::acceptor              m_acceptor;
  std::set<TcpConnection::shptr >           m_connections;
};

} }

#endif 
