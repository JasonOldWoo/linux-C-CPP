#ifndef __DEVICEAGENT_ENDPORT_CORE_H__
#define __DEVICEAGENT_ENDPORT_CORE_H__

#include "../../impl/endport_impl.h"

namespace zl_device_agent {
namespace transport {
namespace core {

typedef void (*tcp_init_handler) (connection_impl*);

class endport : public endport_impl {
public:
	endport() {
	}

	~endport() {
	}

private:
	bool reuse_addr_;
};

}	// namespace core
}	// namespace transport
}	// namespace zl_device_agent
#endif
