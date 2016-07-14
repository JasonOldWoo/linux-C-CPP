#ifndef __BASE_DEF_H__
#define __BASE_DEF_H__

namespace zl_device_agent {

class cbase {
public:
	cbase() {;}
	virtual ~cbase() {;}
};

template <typename _Tp>
struct handler {
	handler() {}
	handler(cbase* obj, _Tp pro)
		: object(obj), proc(pro)
	{}
	cbase* object;
	_Tp proc;
};

// type define
typedef int socket_type;

}	// namespace zl_device_agent

#endif	// __BASE_DEF_H__
