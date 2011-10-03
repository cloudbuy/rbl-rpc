#ifndef RBL_RPC_COMMON_ERRORS_H
#define RBL_RPC_COMMON_ERRORS_H

#include <boost/exception/all.hpp>
#include <boost/system/error_code.hpp>
#include <rpc/proto/BasicProtocol.pb.h>

namespace rubble { namespace rpc {
// ////////////////////////////////////////////////////////////////////////////
  class InvokerError : public boost::system::error_category
  {
    const char * name() const { return "Invoker Error"; }
    std::string message(int ev) const { return "error"; }
  };

  extern InvokerError invoker_error;
  
  struct InvokerException : boost::exception
  {
  };  

  typedef boost::error_info<struct tag_invoker_error_code, 
                            boost::system::error_code> rbl_invoker_error_code;
  
  #define RBL_INVOKER_THROW_EXCEPTION(x) \
    throw InvokerException() \
      << rbl_invoker_error_code(boost::system::error_code(x,invoker_error));
//---------------------------------------------------------------------------//

// ////////////////////////////////////////////////////////////////////////////
  namespace error_codes {
    enum e_code
    {
      RBL_RPC_SUCCESS = 0,
      RBL_RPC_BACKEND_SEALED =1 ,
      RBL_RPC_BACKEND_INITIALIZATION_ERROR=2 ,
      RBL_RPC_BACKEND_IDENTIFIER_COLLISION=3 ,
      RBL_BACKEND_TEARDOWN_ERROR=4,
      RBL_BACKEND_INVOKE_NO_SERVICE_WITH_ORDINAL_ERROR=5,
      RBL_BACKEND_INVOKE_NO_REQUEST_WITH_ORDINAL_ERROR=6,
      RBL_BACKEND_INVOKE_CLIENT_NOT_SUBSCRIBED=7,
      RBL_BACKEND_INVOKE_REQUEST_STRING_PARSE_ERROR=8,
      RBL_BACKEND_INVOKE_RESPONSE_STRING_SERIALIZE_ERROR=9,
      RBL_BACKEND_CLIENT_TARGET_TYPE_MISMATCH=10,
      RBL_BACKEND_CLIENT_SOURCE_TYPE_MISMATCH=11,
      RBL_BACKEND_NOT_ESTABLISHED=12,
      RBL_BACKEND_ALLREADY_ESTABLISHED=13,
      RBL_BACKEND_SUBSCRIBE_NO_SERVICE_WITH_ORDINAL=14,
      RBL_BACKEND_ALLREADY_SUBSCRIBED=15,
      RBL_BACKEND_NOT_SUBSCRIBED=16,
      RBL_BACKEND_LIST_METHOD_NO_SERVICE_WITH_ORDINAL=17,
      RBL_BACKEND_NOT_ACCEPTING_REQUESTS=18
    };
  }

  class RpcBackEndError : public boost::system::error_category
  {
  public:
    const char * name() const { return "RPC Backend"; }
    std::string message( int ev) const { return "error message"; }  
  };

  extern RpcBackEndError rpc_backend_error;

  typedef boost::error_info<struct tag_error_code, boost::system::error_code> 
    rbl_backend_error_code;

  typedef boost::error_info<struct tag_service_error_code, boost::system::error_code>
    rbl_backend_service_error_code;
  
  typedef boost::error_info<struct tag_service_name, std::string>
    rbl_backend_service_name;  

  struct BackEndException :  virtual boost::exception
  {
  };
  #define RBL_BACKEND_THROW_EXCEPTION(x) \
    throw BackEndInitiationException() \
      << rbl_backend_error_code(boost::system::error_code(x, rpc_backend_error);
//---------------------------------------------------------------------------//
} }

#endif 
