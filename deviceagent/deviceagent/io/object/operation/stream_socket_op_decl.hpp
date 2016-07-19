#ifndef __DEVICEAGENT_STREAM_SOCKET_OP_DECL_HPP__
#define __DEVICEAGENT_STREAM_SOCKET_OP_DECL_HPP__
#include "../../../config/core.h"
#include "../../async/proactor.hpp"
#include "../../operation.hpp"
#include <sys/socket.h>
#include <sys/types.h>

namespace zl_device_agent {
typedef async_io::proactor<core_config> core_proactor;
typedef core_proactor* core_proactor_ptr;
typedef core_proactor::reactor_op reactor_op;

struct cbuffer {
	char* msg;
	std::size_t len;
	std::size_t bytes_trans;
	int state;
};

class stream_socket_write_op;
class stream_socket_read_op;
}	// namespace zl_device_agent
#endif	// __deviceegent_stream_socket_op_decl_hpp__
