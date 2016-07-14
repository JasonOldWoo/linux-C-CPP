#ifndef __DEVICEAEGNT_ENDPORT_IMPL_H__
#define __DEVICEAEGNT_ENDPORT_IMPL_H__

#include "connection_impl.h"

class endport_impl {
public:
	/* 创建资源\句柄 */
	virtual void init_io() = 0;
	virtual void listen() = 0;
	virtual void stop_listening() = 0;

protected:
	virtual void init(connection_impl*) = 0;
};

#endif	//  __DEVICEAEGNT_ENDPORT_IMPL_H__
