#ifndef RBL_RPC_SERVER_RPC_COMMON_H
#define RBL_RPC_SERVER_RPC_COMMON_H
#include <rpc/common/rpc_common.h>
#include <rpc/proto/BasicProtocol.pb.h>

namespace rubble { namespace rpc {
  class ClientCookieBase;
  class ClientData;

  class ServiceBase
  {
  public:
    typedef boost::shared_ptr<ServiceBase> shp;

    virtual ~ServiceBase(){};

    virtual void init(boost::system::error_code & ec) =0;
    virtual void teardown(boost::system::error_code & ec)=0;
    virtual void subscribe(ClientCookie & client_cookie, ClientData & cd,
      std::string *, std::string *) = 0;
    virtual void unsubscribe(ClientCookie & client_cookie, ClientData & cd) = 0;
    virtual bool contains_function_at_ordinal(boost::uint16_t ordinal) = 0;
    virtual void dispatch(ClientCookie & client_cookie, ClientData & cd)=0;
    virtual const char * name() = 0;
    virtual void produce_method_list(basic_protocol::ListMethodsResponse & lmr) = 0;

    common::ordinal_type ordinal() { return m_ordinal; }
    void set_ordinal(common::ordinal_type ordinal) { m_ordinal = ordinal; }
  private:
    common::ordinal_type m_ordinal;
  };

  template<typename T>
  void ff_produce_method_list(T & dispatch_table, basic_protocol::ListMethodsResponse & lmr)
  {
    int sz = dispatch_table.size();

    rubble::rpc::basic_protocol::MethodEntry * me;

    for(int i =0; i < sz; ++i)
    {
      me = lmr.add_methods();

      const typename T::entry_type * e = dispatch_table.EntryAtordinal(i);

      BOOST_ASSERT_MSG(e != NULL, 
        "THERE SHOULD BE NO HOLE IN THE DISPATCH TABLE");
      me->set_service_name( e->name().c_str());
      me->set_service_ordinal( e->ordinal());
    }   
  }
 
} } 

#include <rpc/proto/BasicProtocol-server.rblrpc.h>
#endif 
