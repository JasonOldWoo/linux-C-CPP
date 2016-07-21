#ifndef __DEVICEAGENT_ENDPORT_UDX_H__
#define __DEVICEAGENT_ENDPORT_UDX_H__
#include "../../impl/endport_impl.h"

namespace zl_device_agent {
namespace transport {
namespace udx {

typedef void (*init_handler) (connection_impl*);

class endport : public endport_impl {
public :
	endport() {
	}

	~endport() {
	}

	void set_init_handler() {
	}
};
}	// namespace udx
}	// namespace transport
}	// namespace zl_device_agent

#endif	// __DEVICEAGENT_ENDPORT_UDX_H__
