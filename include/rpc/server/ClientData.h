#ifndef RBL_RPC_CLIENT_DATA_H
#define RBL_RPC_CLIENT_DATA_H
#include <rpc/common/rpc_common.h>
#include <rpc/proto/BasicProtocol.pb.h>
#include <boost/thread.hpp>
#include <boost/scoped_array.hpp>

namespace rubble { namespace rpc {
  struct buffer
  {
    buffer()
      : buf_size(RBL_RPC_CONF_INITIAL_BUFFER_SIZE),
        buf(new boost::uint8_t[RBL_RPC_CONF_INITIAL_BUFFER_SIZE])
    {
       
    }
    
    boost::uint8_t * resize_buffer(std::size_t new_size)
    {
      buf.reset(new boost::uint8_t[new_size]);
      buf_size=new_size;
      return buf.get();
    }
    boost::uint8_t * get()
    {
      return buf.get();
    }

    std::size_t                             buf_size;
    boost::scoped_array<boost::uint8_t>     buf;
  };

  class ClientData
  {
  public:
    typedef boost::shared_ptr<ClientData> shp;

    ClientData() 
      : m_flags(0),
        m_rpc_active(false),
        m_should_disconect(false)  {}

    
    boost::uint32_t & flags() { return m_flags; }
    
    basic_protocol::ClientRequest & request() { return m_request; }
    basic_protocol::ClientResponse & response() { return m_response;  }

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
    
    void name(const std::string & name_in) { m_name = name_in; }
    const std::string & name() { return m_name; }

    void error_code(boost::system::error_code & ec)
      { m_ec = ec; }
    boost::system::error_code & error_code()
      { return m_ec; }

    void request_disconect()
      { m_should_disconect = true;}
    bool should_disconect()
      { return m_should_disconect; }
  private:
    basic_protocol::ClientRequest           m_request;
    basic_protocol::ClientResponse          m_response;

    std::string                             m_name;
    boost::system::error_code               m_ec;   
    boost::uint32_t                         m_flags;
    void *                                  m_io_object_ptr;
    bool                                    m_rpc_active;
    bool                                    m_should_disconect;
  };
} }
#endif
