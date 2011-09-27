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
      RBL_RPC_BACKEND_SEALED,
      RBL_RPC_BACKEND_INITIALIZATION_ERROR,
      RBL_RPC_BACKEND_IDENTIFIER_COLLISION,
      RBL_BACKEND_TEARDOWN_ERROR,
      RBL_BACKEND_INVOKE_NO_SERVICE_WITH_ORDINAL_ERROR,
      RBL_BACKEND_INVOKE_NO_REQUEST_WITH_ORDINAL_ERROR,
      RBL_BACKEND_INVOKE_CLIENT_NOT_SUBSCRIBED,
      RBL_BACKEND_INVOKE_REQUEST_STRING_PARSE_ERROR,
      RBL_BACKEND_INVOKE_RESPONSE_STRING_SERIALIZE_ERROR,
      RBL_BACKEND_CLIENT_TARGET_TYPE_MISMATCH,
      RBL_BACKEND_CLIENT_SOURCE_TYPE_MISMATCH,
      RBL_BACKEND_NOT_ESTABLISHED,
      RBL_BACKEND_ALLREADY_ESTABLISHED,
      RBL_BACKEND_SUBSCRIBE_NO_SERVICE_WITH_ORDINAL,
      RBL_BACKEND_ALLREADY_SUBSCRIBED,
      RBL_BACKEND_NOT_SUBSCRIBED,
      RBL_BACKEND_LIST_METHOD_NO_SERVICE_WITH_ORDINAL,
      RBL_BACKEND_NOT_ACCEPTING_REQUESTS
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
