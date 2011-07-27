#ifndef RBL_RPC_CLIENT_DATA_H
#define RBL_RPC_CLIENT_DATA_H
#include <rpc/common/rpc_common.h>
#include <rpc/proto/BasicProtocol.pb.h>

#include <boost/scoped_array.hpp>

namespace rubble { namespace rpc {

  class ClientData
  {
  public:
    ClientData() 
      : m_flags(0),
        m_buffer_size(RBL_RPC_CONF_INITIAL_BUFFER_SIZE),
        m_buffer(new boost::uint8_t[RBL_RPC_CONF_INITIAL_BUFFER_SIZE]),
        m_rpc_active(false) {}

    boost::uint8_t * resize_buffer(std::size_t new_size)
    {
      m_buffer.reset(new boost::uint8_t[new_size]);
      m_buffer_size=new_size;
      return m_buffer.get();
    }
 
    boost::uint8_t * buffer() { return m_buffer.get(); } 
    boost::uint32_t & flags() { return m_flags; }
    
    basic_protocol::ClientRequest & request() { return m_request; }
    basic_protocol::ClientResponse & response() { return m_response;  }

    std::size_t buffer_size() { return m_buffer_size; }

    void start_rpc() 
    { 
      BOOST_ASSERT_MSG( (! m_rpc_active), 
        "RPC SHOULD BE INACTIVE, IN ORDER TO START A NEW ONE");
      m_rpc_active = true; 
    }
    void end_rpc() 
    { 
      BOOST_ASSERT_MSG( m_rpc_active, 
        "RPC SHOULD BE ACTIVE, IN ORDER TO STOP IT!");
      m_rpc_active = false;
    };
    bool is_rpc_active() { return m_rpc_active; } 
  private:
    basic_protocol::ClientRequest           m_request;
    basic_protocol::ClientResponse          m_response;

    std::string                             name;
    boost::system::error_code               n_ec;   
    std::size_t                             m_buffer_size;
    boost::scoped_array<boost::uint8_t>     m_buffer;
    boost::uint32_t                         m_flags;
    void *                                  m_io_object_ptr;
    bool                                    m_rpc_active;
    
  };
  typedef boost::shared_ptr<ClientData> ClientData_shp;
} }
#endif
