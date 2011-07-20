#ifndef RBL_RPC_CLIENT_RPC_COMMON_H
#define RBL_RPC_CLIENT_RPC_COMMON_H
 
#include "common/rpc_common.h"

namespace rubble { namespace rpc {
  class ClientConnection; 
 
  class ClientServiceSubscription
  {
  public:
    ClientServiceSubscription(const boost::int16_t & method_count);

    void service_ordinal(boost::int16_t service_ordinal)
    {
      m_client_request.set_service_ordinal(service_ordinal);
    }
    boost::uint16_t service_ordinal() { return m_client_request.service_ordinal(); } 

    bool dispatch() 
    {
      return true;
    } 
  protected:    
    basic_protocol::ClientRequest m_client_request;
    ClientConnection & m_client_connection;
    common::OidContainer<common::Oid, boost::int16_t> m_service_method_map;
  };
} }   

#include <rpc/proto/BasicProtocol-client.rblrpc.h>

namespace rubble { namespace rpc {
  
  class ClientServiceFactory
  {
  public:
    ClientServiceFactory(ClientConnection & cc) 
      : m_client_connection(cc) 
    {
      rubble:rpc::basic_protocol::HelloRequest    req;
      rubble::rpc::basic_protocol::HelloResponse  res;
      
      req.set_source_type(m_client_connection.source_connection_type());
      req.set_expected_target(m_client_connection.destination_connection_type());
      req.set_node_name(m_client_connection.name());

      m_client_request.set_service_ordinal(0);
      m_client_request.set_request_ordinal(0);
      req.SerializeToString(m_client_request.mutable_request_string());

      m_client_connection.rpc_call_dispatch(m_client_request, m_client_response);
      
    }
    
  private:
    basic_protocol::ClientRequest m_client_request;
    basic_protocol::ClientResponse m_client_response;
    ClientConnection & m_client_connection;
    common::OidContainer<common::Oid, ClientServiceSubscription * > m_service_map;
  };

} }
#endif 
