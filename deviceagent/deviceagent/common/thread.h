#ifndef __DEVICE_AGENT_THREAD_H__
#define __DEVICE_AGENT_THREAD_H__
#include <thread>

namespace zl_device_agent {
namespace lib {
//#ifdef DEVICE_AGENT_ENABEL_CPP11
	using std::thread;
//#else
	// TODO 统一封装bt::thread为标准接口
	//using bt::thread;
//#endif
}	// namespace thread
}	// namespace zl_device_agent
#endif	// __DEVICE_AGENT_THREAD_H__
