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
        m_buffer(new boost::uint8_t[RBL_RPC_CONF_INITIAL_BUFFER_SIZE]) {}

    boost::uint8_t * resize_buffer(std::size_t new_size)
    {
      m_buffer.reset(new boost::uint8_t[new_size]);
      m_buffer_size=new_size;
      return m_buffer.get();
    }
    
    boost::uint8_t * buffer() { return m_buffer.get(); } 
    boost::uint32_t & flags() { return m_flags; }
    basic_protocol::ClientRequest request() { return m_request; }

    std::size_t buffer_size() { return m_buffer_size; }
  private:
    basic_protocol::ClientRequest m_request;
    std::string name;
   
    std::size_t m_buffer_size;
    boost::scoped_array<boost::uint8_t> m_buffer;
    boost::uint32_t m_flags;
    void * m_io_object_ptr;
  };
  typedef boost::shared_ptr<ClientData> ClientData_shp;
} }
#endif
