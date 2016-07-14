#ifndef __DEVICEAGENT_CONNECTION_CORE_H__
#define __DEVICEAGENT_CONNECTION_CORE_H__

#include "../../impl/connection_impl.h"

namespace zl_device_agent {
namespace transport {
namespace core {

typedef int (cbase::*init)(void);
typedef void (cbase::*socket_init)(connection_impl*, int);

typedef struct init_handler : public handler<init> {
} init_handler;

typedef struct socket_init_handler : public handler<socket_init> {
} socket_init_handler;

// default sync mode
class connection : public connection_impl {
public:

protected:
	void set_socket_init_handler(socket_init_handler);
	int init_io(bool is_server);
	/*
	void set_uri(uri* u);
	*/
	void pre_init(init_handler callback);
	void post_init(init_handler callback);
	int handle_init(init_handler callback);
	int cancel_socket();
	int shutdown();

private:
	int socket_;
	socket_init_handler socket_init_handler_;
	init_handler init_handler_;
};

}	// namespace core
}	// namespace transport
}	// namespace zl_device_agent

#endif	// __DEVICEAGENT_CONNECTION_CORE_H__
