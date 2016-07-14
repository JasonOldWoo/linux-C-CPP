#include <iostream>
#ifdef WEBCLIENTUDX_USE_CPP11
#include <functional>
#else
#endif

namespace zl_device_agent {

namespace lib {
#ifdef WEBCLIENTUDX_USE_CPP11
	using std::function;
#else
#endif
}

class endport_impl;

/* abstraction */
class endport {
public:
	virtual ~endport();

protected:
	virtual endport_impl* get_impl() = 0;

private:
	endport_impl* impl_;
};

// client
class client : public endport {
public:
	endport_impl* get_impl();
};

// server
class server : public endport {
public:
	endport_impl* get_impl();
};

/* oncrete implementation */
class endport_impl {
public:
	virtual void set_message_handler(/*....*/) = 0;
};

// tcp
class con_impl : public endport_impl {
public:
	void set_message_handler(/*....*/);
};

// p2p
class udx_impl : public endport_impl {
public:
	void set_message_handler(/*....*/);
};

}	// zl_device_agent

/* service */

class proxy_client : public zl_device_agent::client {
};

class proxy_server : public zl_device_agent::server {
};

class device_agent {
};

/* demo */
int main(int argc, char* argv[])
{
	std::cout << "argc: " << argc
		<< ", " << "argv[0]: "
		<< argv[0] << std::endl;

	return 0;
}
