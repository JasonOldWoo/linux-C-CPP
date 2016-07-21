#ifndef __DEVICEAGENT_TCP_PROTOCOL_H__
#define __DEVICEAGENT_TCP_PROTOCOL_H__

#include "protocol.h"
#include <sys/socket.h>

namespace zl_device_agent {
class tcp_protocol : public protocol {
public:
	explicit tcp_protocol()
		: protocol(AF_INET, SOCK_STREAM)
	{}
};
}	// zl_device_agent

#endif	// __deviceagent_tcp_protocol_h__
