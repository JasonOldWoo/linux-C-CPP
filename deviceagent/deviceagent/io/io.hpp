#include "../basedef.h"

namespace zl_device_agent {
template <typename config, typename type>
class io : public cbase {
	typedef typename type::protocol protocol;
};
}	// namespace zl_device_agent
