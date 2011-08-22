#ifndef RBL_RPC_CLIENT_SERVICE_FACTORY_H
#define RBL_RPC_CLIENT_SERVICE_FACTORY_H

#include <rpc/backend/BackEndBase.h>
#include <rpc/invoker/InvokerBase.h>
#include <rpc/proto/BasicProtocol-client.rblrpc.h>
#include <rpc/client/ClientBase.h>
#include <boost/scoped_ptr.hpp>


namespace rubble {
namespace rpc {
  class BasicProtocolMethodMap : public ClientServiceBase::t_service_method_map
  {
  public:
    BasicProtocolMethodMap()
    {
      SetEntry(common::Oid("hello",0),                   
        BASIC_PROTOCOL_HELLO_ORDINAL          );
      SetEntry(common::Oid("list_services",1),           
        BASIC_PROTOCOL_LIST_SERVICES_ORDINAL  );
      SetEntry(common::Oid("rpc_subscribe_service",2),   
        BASIC_PROTOCOL_RPC_SUBSCRIBE_ORDINAL  );
      SetEntry(common::Oid("rpc_unsubscribe_service",3), 
        BASIC_PROTOCOL_RPC_UBSUBSCRIBE_ORDINAL);
      SetEntry(common::Oid("list_methods",4),            
        BASIC_PROTOCOL_LIST_METHODS_ORDINAL   );
    }
  };


class ServiceClientFactory : public boost::noncopyable
{
public:
  typedef boost::scoped_ptr<ServiceClientFactory> scptr;

  ServiceClientFactory( std::string name, InvokerBase & invoker,
                        basic_protocol::SourceConnectionType source_type,
                        basic_protocol::DestinationConnectionType dest_type)
    : m_name(name),
      m_invoker(invoker),
      bpc(new basic_protocol::basic_protocol_client(m_invoker))
  {
    if( ! m_invoker.is_useable() ) 
      throw BackEndException();
    
    bpc->set_service_ordinal(0);
    bpc->remap_ordinals(m_method_map);

    basic_protocol::HelloRequest hreq;
    basic_protocol::HelloResponse hres;
   
    hreq.set_source_type(source_type);
    hreq.set_expected_target(dest_type);
    hreq.set_node_name(m_name);
  
    bpc->hello(hreq, hres); 
    if( hres.error_type() != basic_protocol::NO_HELLO_ERRORS)
      throw BackEndException();

    basic_protocol::ListServicesRequest lreq;
    basic_protocol::ListServicesResponse lres;
    
    bpc->list_services(lreq,lres);
    
    for(int i=0; i< lres.services_size(); ++i)
    {
      const basic_protocol::ServiceEntry e = 
        lres.services(i);
      m_services.SetEntry(
        common::Oid(e.service_name(), e.service_ordinal()),
        ClientServiceBase::shptr());
    }
  }

  template<typename RT>
  typename RT::shptr get_service( const std::string & service_name)
  {
    common::OidContainer<common::Oid, ClientServiceBase::shptr>::entry_type* e 
      = m_services.EntryWithName(service_name);

    if ( e == NULL)
      throw BackEndException();
    
    if( (*e).entry().get() == NULL)
      throw BackEndException();
    
    return boost::static_pointer_cast<RT>( (*e).entry());
  }

  template<typename RT>
  typename RT::shptr subscribe_service(
    const std::string & service_name, 
    google::protobuf::Message * subscription_request_data = NULL,
    google::protobuf::Message * subscription_response_data = NULL)
  {
    common::OidContainer<common::Oid, ClientServiceBase::shptr>::entry_type* e 
        = m_services.EntryWithName(service_name);
    
    if( e == NULL) // the named service does not exist
      throw BackEndException();

    if((*e).entry().get() != NULL) // subscription should only occur once.
      throw BackEndException();
   
    if((*e).ordinal() >= service_count()) // this should never occur
      throw BackEndException();      

    basic_protocol::SubscribeServiceRequest req;
    basic_protocol::SubscribeServiceResponse res;
    
    req.set_service_ordinal( (*e).ordinal());
  
    if(subscription_request_data != NULL) 
    {
      subscription_request_data->SerializeToString(
        req.mutable_subscribe_request_string());
    }
    bpc->rpc_subscribe_service(req,res);
    
    if(res.error() != basic_protocol::NO_SUBSCRIBE_SERVICE_ERROR)
      throw BackEndException();

    if(! res.subscribe_result_string().empty())
    {
      if(subscription_response_data == NULL)
        throw BackEndException();

      subscription_response_data->ParseFromString(
        res.subscribe_result_string()
      );
    }
    (*e).entry().reset(new RT(m_invoker));
  
    // populate the method map
    basic_protocol::ListMethodsRequest  lm_req;
    basic_protocol::ListMethodsResponse lm_res;
    lm_req.set_service_ordinal((*e).ordinal());
    bpc->list_methods(lm_req, lm_res);
    
    if( lm_res.error() != basic_protocol::NO_LIST_METHOD_ERROR )
      throw BackEndException();

    for(int i=0; i < lm_res.methods_size(); ++i)
    {
      const basic_protocol::MethodEntry & me = lm_res.methods(i);
      
      boost::uint16_t * ordinal = 
        (*e).entry()->m_service_method_map[me.service_name()];
      
      *ordinal = me.service_ordinal();
    }

    // make sure no method/ordinals are uninitialized in the method map
    for(int i = 0; i < (*e).entry()->m_service_method_map.size(); ++i)
    {
      boost::uint16_t * ordinal = 
        (*e).entry()->m_service_method_map[i];
      
      if(*ordinal == -1)
        throw BackEndException();
    }
    (*e).entry()->set_service_ordinal(e->ordinal());
    (*e).entry()->set_is_subscribed(true);
    return boost::static_pointer_cast<RT>( (*e).entry());
  }

  void unsubscribe_service(const std::string & service_name)
  {
    common::OidContainer<common::Oid, ClientServiceBase::shptr>::entry_type* e 
      = m_services.EntryWithName(service_name);

    if ( e == NULL)
      throw BackEndException();
    
    if( (*e).entry().get() == NULL)
      throw BackEndException();
    
    basic_protocol::UnsubscribeServiceRequest req;
    basic_protocol::UnsubscribeServiceResponse res;

    req.set_service_ordinal( (*e).ordinal());
   
    bpc->rpc_unsubscribe_service(req,res);
  
    if(res.error() != basic_protocol::NO_SUBSCRIBE_SERVICE_ERROR)
      throw BackEndException();

    (*e).entry()->set_is_subscribed(false); 
  }

  boost::uint16_t service_count()
  {
    return m_services.size();
  }
  
  bool has_service_with_name(const std::string & str)
  {
    ClientServiceBase::shptr * tmp = m_services[str];
    
    if( tmp == NULL)
      return false;
    else
      return true;   
  }
private:
  InvokerBase &   m_invoker;
  std::string     m_name;
  basic_protocol::basic_protocol_client::scptr bpc;
  common::OidContainer<common::Oid, ClientServiceBase::shptr> m_services;
  static BasicProtocolMethodMap m_method_map;
};

} } 

#endif
