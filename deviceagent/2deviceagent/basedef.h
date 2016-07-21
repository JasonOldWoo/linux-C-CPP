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

// boundary
#define MAX_IOVEC_NUM 16
#define MAX_CBUFF_SIZ MAX_IOVEC_NUM

}	// namespace zl_device_agent

#endif	// __BASE_DEF_H__
