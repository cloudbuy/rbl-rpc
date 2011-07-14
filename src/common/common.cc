#include <common/common.h>

namespace rubble { namespace common {
  template class OidConstrainedString<char,32>;
  template class OidType<OidName, ordinal_type>;  
} }
