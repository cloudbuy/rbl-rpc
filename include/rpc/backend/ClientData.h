#ifndef RBL_RPC_CLIENT_DATA_H
#define RBL_RPC_CLIENT_DATA_H
#include <rpc/common/rpc_common.h>
#include <rpc/proto/BasicProtocol.pb.h>
#include <boost/thread.hpp>
#include <boost/shared_array.hpp>
#include <boost/thread.hpp>

#include <set>

namespace rubble { namespace rpc {
  struct Buffer
  {
    Buffer()
      : buf_size(RBL_RPC_CONF_INITIAL_BUFFER_SIZE),
        buf(new boost::uint8_t[RBL_RPC_CONF_INITIAL_BUFFER_SIZE])
    {
       
    }
   
    std::size_t size()
    {
      return buf_size;
    }
 
    boost::uint8_t * resize(std::size_t new_size)
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
    typedef boost::shared_ptr<ClientData> shptr;
    typedef boost::weak_ptr<ClientData>   wptr;
    typedef ClientData *                  ptr;

    ClientData() 
      : m_flags(0),
        m_rpc_active(false),
        m_should_disconect(false),
        m_client_established(false)  {}

    
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
   
    void establish_client()
    { 
      BOOST_ASSERT_MSG( (! m_client_established),  "CANNOT ESTABLISH A CLIENT TWICE");
      m_client_established = true;
    }
    bool is_client_established()
      { return m_client_established; }
  private:
    basic_protocol::ClientRequest           m_request;
    basic_protocol::ClientResponse          m_response;

    std::string                             m_name;
    boost::system::error_code               m_ec;   
    boost::uint32_t                         m_flags;
    
    bool                                    m_rpc_active;
    bool                                    m_should_disconect;
    bool                                    m_client_established;
  };

  class ConnectedClientsSet  {
  public:
    typedef std::set<ClientData::wptr> set_type;

    std::pair<set_type::iterator, bool> insert (const set_type::value_type &x)
    {
      boost::lock_guard<boost::mutex> lock(m_mutex); 
      
      return m_set.insert(x);
    }
    set_type::size_type erase (const set_type::key_type & x)
    {
      boost::lock_guard<boost::mutex> lock(m_mutex); 
      
      return m_set.erase(x);
    }
    set_type::size_type size() const
    {
      boost::lock_guard<boost::mutex> lock(
        const_cast<ConnectedClientsSet *>(this)-> m_mutex); 
    
      return m_set.size();
    }
  private: 
    set_type m_set;
    boost::mutex m_mutex;
  };
} }
#endif
