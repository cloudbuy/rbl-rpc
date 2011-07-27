#ifndef RBL_RPC_COMMON_ERRORS_H
#define RBL_RPC_COMMON_ERRORS_H

#include <boost/exception/all.hpp>
#include <boost/system/error_code.hpp>

namespace rubble { namespace rpc {



  namespace error_codes {
    enum e_code
    {
      RBL_RPC_SUCCESS = 0,
      RBL_RPC_BACKEND_SEALED,
      RBL_RPC_BACKEND_INITIALIZATION_ERROR,
      RBL_RPC_BACKEND_IDENTIFIER_COLLISION,
      RBL_BACKEND_TEARDOWN_ERROR
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
} }

#endif 
