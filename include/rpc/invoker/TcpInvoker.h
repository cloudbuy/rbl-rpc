#ifndef RBL_RPC_TCP_INVOKER_H
#define RBL_RPC_TCP_INVOKER_H
#include <rpc/invoker/InvokerBase.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <boost/asio.hpp>

namespace rubble { namespace rpc {
  class TcpInvokerException
  {
  };

  struct TcpInvoker : public InvokerBase
  {
    TcpInvoker(const std::string & str, short port)
      : m_io_service(),
        m_socket(m_io_service),
        m_endpoint(boost::asio::ip::address::from_string(str), port),
        m_useable(false)

    {
      m_socket.connect(m_endpoint, m_error_code);
      
      if(!m_error_code)
        m_useable = true;
    }

    void reset()
    {
      m_client_data->request().Clear();
      m_client_data->response().Clear();
      m_client_data->error_code().clear();
      BOOST_ASSERT_MSG( m_client_data->is_rpc_active() == false, 
        "THE FLAG THAT REPRESENTS ACTIVE "
        "RPC SHOULD NOT BE SET WHEN RESETING AN OBJECT FOR RPC");
    }

    void invoke()
    {
      boost::uint32_t flag_return;
      boost::uint32_t msg_size_return;

      boost::uint32_t msg_size = 8 + request().ByteSize();
      
      if(msg_size > m_buffer.size() )
        m_buffer.resize(msg_size);
      
      google::protobuf::io::ArrayOutputStream aos(m_buffer.get(),m_buffer.size());
      google::protobuf::io::CodedOutputStream cos(&aos);
      
      cos.WriteLittleEndian32(0); // FOR RPC FLAGS -- UNUSED ATM
      cos.WriteLittleEndian32(msg_size);
      request().SerializeToCodedStream(&cos);
      std::size_t transfered_count = 
        boost::asio::write(m_socket, 
          boost::asio::buffer(m_buffer.get(), msg_size),m_error_code);

      if(!m_error_code)
      {
        boost::asio::read(m_socket, boost::asio::buffer(m_buffer.get(), 8),m_error_code);
        if(!m_error_code)
        {
        
          google::protobuf::io::CodedInputStream cis(m_buffer.get(),m_buffer.size());
          
          cis.ReadLittleEndian32( & flag_return);
          cis.ReadLittleEndian32( & msg_size_return);
          
          if(msg_size_return < m_buffer.size())
            m_buffer.resize(msg_size_return);
          
          transfered_count = boost::asio::read(m_socket, 
            boost::asio::buffer(m_buffer.get(), msg_size_return),m_error_code);
        }
        else
          std::cout << "error" << std::endl;
      }
      else
        std::cout << "error" << std::endl;
    }
    
    void operator() ()
    {
      // no-op as the invocation is synchronous inline imperetive entirely 
      // coverd in this->inoke()
    }

    void after_post()
    {
      // no-op for the same reasons as the above.
    }

    bool is_useable()
    {
      return m_useable;
    }
  
    bool                            m_useable;
    boost::asio::io_service         m_io_service;
    boost::asio::ip::tcp::socket    m_socket;
    boost::asio::ip::tcp::endpoint  m_endpoint;
    boost::system::error_code       m_error_code; 
    Buffer                          m_buffer;
  };

} }
#endif 
