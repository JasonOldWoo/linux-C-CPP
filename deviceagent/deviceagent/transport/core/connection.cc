#include "connection.h"

namespace zl_device_agent {
namespace transport {
namespace core {

void connection::set_socket_init_handler(socket_init_handler h) {
	socket_init_handler_ = h;
}

int connection::init_io(bool is_server) {
}

}	// namespace core
}	// namespace transport
}	// namespace zl_device_agent
