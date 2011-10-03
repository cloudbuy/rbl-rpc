#ifndef RBL_COMMON_H
#define RBL_COMMON_H

#include <string>
#include <vector>

#include <boost/noncopyable.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>


#include "oid_container.h"

namespace rubble { namespace common {
  typedef boost::uint16_t ordinal_type;
  typedef OidConstrainedString<char, 32> OidName;
  typedef OidType<OidName, ordinal_type> Oid; 
} }

namespace rubble {
  enum LOG_SERVICE_TYPE
  {
    MARSHALL=0,
    RELAY,
    SOURCE
  };
}

#include "logging.h"
#endif 
