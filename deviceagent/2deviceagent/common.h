#ifndef __DEVICEAGENT_COMMON_H__
#define __DEVICEAGENT_COMMON_H__
#ifdef _DEVICEAGENT_CPP11_ENABLE_
#include <functional>
#endif
#include <deque>
#include <list>
namespace zl_device_agent {
namespace lib {
	using std::deque;
	using std::list;
	// function
#ifdef _DEVICEAGENT_CPP11_ENABLE_
#else
#endif	// _DEVICEAGENT_CPP11_ENABLE_
}	// namespace lib
}	// namespace zl_device_agent
#endif	// __DEVICEAGENT_COMMON_H__
