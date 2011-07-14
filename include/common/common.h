#include "oid_container.h"

namespace rubble { namespace common {
  typedef boost::uint16_t ordinal_type;
  typedef OidConstrainedString<char, 32> OidName;
  typedef OidType<OidName, ordinal_type> Oid; 
} }
