#ifndef __DEVICEAGENT_PROTOCOL_H__
#define __DEVICEAGENT_PROTOCOL_H__

namespace zl_device_agent {

class protocol {
public:
	explicit protocol(int f, int l)
		: family(f), level(l)
	{}
	const int family;
	const int level;
};

}	// namespace zl_device_agent

#endif	// __deviceagent_protocol_h__
