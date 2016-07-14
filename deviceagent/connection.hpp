#include <memory>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <system_error>

namespace zl_device_agent {
typedef std::weak_ptr<void> connection_hdl;
typedef std::function<void(std::error_code const&)> init_handler;
typedef std::function<void(std::error_code const&, size_t)> read_handler;
typedef std::function<void(std::error_code const&)> write_handler;
typedef std::function<void(std::error_code const&)> timer_handler;
typedef std::function<void(std::error_code const&)> shutdown_handler;
typedef std::function<void()> interrupt_handler;
typedef std::function<void()> dispatch_handler;

struct buffer {
	buffer(char const* b, size_t l) : buf(b), len(l) {;}

	char const* buf;
	size_t len;
};

namespace error {
enum value {
	general = 1,
	pass_through,
	invalid_num_bytes,
	double_read,
	operation_aborted,
	operation_not_supported,
	eof,
	tls_short_read,
	timeout,
	action_after_shutdown,
	tls_error,
};

class category : public std::error_category {
public:
	category() {}

	char const* name() const noexcept {
		return "zl_device_agent";
	}
	std::string message(int value) const {
		switch (value) {
		case general:
			return "generic transport policy error";
		case pass_through:
			return "underlying transport error";
		case invalid_num_bytes:
			return "async_read_at_least call requested more bytes than buffer can store";
		case operation_aborted:
			return "the operation was aborted";
		case operation_not_supported:
			return "the operation is not supported by this transport";
		case eof:
			return "end of file";
		case timeout:
			return "timer expired";
		case action_after_shutdown:
			return "a transport action was requested after shutdown";
		case tls_error:
			return "generic tls related error";
		default:
			return "unknown";
		}
	}
};

inline std::error_category const& get_category() {
	static category instance;
	return instance;
}

inline std::error_code make_error_code(error::value e) {
	return std::error_code(static_cast<int>(e), get_category());
}
}	// namespace error

namespace iostream {
typedef std::function<std::error_code(connection_hdl, char const*, size_t)> write_handler;
typedef std::function<std::error_code(connection_hdl, std::vector<buffer> const& bufs)> vector_write_handler;
typedef std::function<std::error_code(connection_hdl)> shutdown_handler;
struct timer {
	void cancel() {;}
};

namespace error {
enum value {
	general = 1,
	invalid_num_bytes,
	double_read,
	output_stream_required,
	bad_stream,
};

class category : public std::error_category {
public:
	category() {}
	char const* name() const noexcept {
		return "zl_device_agent.iostream";
	}

	std::string message(int value) const {
		switch (value) {
		case general:
			return "generic iostream transport policy error";
		case invalid_num_bytes:
			return "async_read_at_least call requested more bytes than buffer can store";
		case double_read:
			return "async read already in progress";
		case output_stream_required:
			return "an output stream to be set before async_write can be used";
		case bad_stream:
			return "a stream opeartion returned ios::bad";
		default:
			return "unknown";
		}
	}
};

inline std::error_category const & get_category() {
	static category instance;
	return instance;
}

inline std::error_code make_error_code(value e) {
	return std::error_code(static_cast<int>(e), get_category());
}
}	// namespace error

template <typename config>
class connection : public std::enable_shared_from_this<connection<config> > {
public:
	typedef connection<config> type;
	typedef std::shared_ptr<type> ptr;
	typedef typename config::concurrency_type concurrency_type;

	typedef typename concurrency_type::scoped_lock_type scoped_lock_type;
	typedef typename concurrency_type::mutex_type mutex_type;

	typedef std::shared_ptr<timer> timer_ptr;

	explicit connection(bool is_server)
		: is_server_(is_server)
		, output_stream_(NULL)
		, reading_(false)
		, remote_endpoint_("iostream transport")
	{
		std::cout << "iostream con constructor" << std::endl;
	}

	ptr get_shared() {
		return type::shared_from_this();
	}

	void register_ostream(std::ostream* o) {
		scoped_lock_type lock(read_mutex_);
		output_stream_ = o;
	}

	/*
	// set uri hook
	void set_uri(uri_ptr) {}
	*/

	// overload stream input operator
	friend std::istream & operator >> (std::istream& in, type& t) {
		scoped_lock_type lock(t.read_mutex_);
		t.read(in);
		return in;
	}

	// manual input supply (read some)
	size_t read_some(char const* buf, size_t len) {
		scoped_lock_type lock(read_mutex_);
		return this->read_some_impl(buf, len);
	}

	// manual input supply (read all)
	size_t read_all(char const* buf, size_t len) {
		scoped_lock_type lock(read_mutex_);
		size_t total_read = 0;
		size_t temp_read = 0;

		do {
			temp_read = this->read_some_impl(
				buf + total_read, len - total_read);
			total_read += temp_read;
		} while (temp_read != 0 && total_read < len);

		return total_read;
	}

	// useeggful
	size_t readsome(char const* buf, size_t len) {
		return this->read_some(buf, len);
	}

	// signal eof
	void eof() {
		scoped_lock_type lock(read_mutex_);

		if (reading_) {
			// TODO error code implement
			complete_read(/*....*/);
		}
	}

	// signal transport error
	void fatal_error() {
		scoped_lock_type lock(read_mutex_);

		if (reading_) {
			complete_read(make_error_code(zl_device_agent::error::pass_through));
		}
	}

	void set_remote_endpoint(std::string value) {
		remote_endpoint_ = value;
	}

	std::string get_remote_endpoint() const {
		return remote_endpoint_;
	}

	connection_hdl get_handle() const {
		return connection_hdl_;
	}

	timer_ptr set_timer(long, timer_handler) {
		return timer_ptr();
	}

	void set_write_handler(write_handler h) {
		write_handler_ = h;
	}

	void set_vector_write_handler(vector_write_handler h) {
		vector_write_handler_ = h;
	}

	void set_shutdown_handler(zl_device_agent::shutdown_handler h) {
		shutdown_handler_ = h;
	}

protected:
	void init(init_handler handler) {
		handler(std::error_code());
	}

	void async_read_at_least(size_t num_bytes, char* buf, size_t len,
		zl_device_agent::read_handler handler) {
		if (num_bytes > len) {
			handler(make_error_code(error::invalid_num_bytes), size_t(0));
			return ;
		}
		if (true == reading_) {
			handler(make_error_code(error::double_read), size_t(0));
			return ;
		}
		if (0 == num_bytes || 0 == len) {
			handler(std::error_code(), size_t(0));
			return ;
		}

		buf_ = buf;
		len_ = len;
		bytes_needed_ = num_bytes;
		read_handler_ = handler;
		cursor_ = 0;
		reading_ = true;
	}

	void async_write(char const* buf, size_t len, zl_device_agent::write_handler handler) {
		std::error_code ec;

		if (output_stream_) {
			output_stream_->write(buf, len);

			if (output_stream_->bad()) {
				ec = make_error_code(error::bad_stream);
			}
		} else if (write_handler_) {
			ec = write_handler_(connection_hdl_, buf, len);
		} else {
			ec = make_error_code(error::output_stream_required);
		}

		handler(ec);
	}

	void async_write(std::vector<buffer> const& bufs, zl_device_agent::write_handler handler) {
		std::error_code ec;
		if (output_stream_) {
			std::vector<buffer>::const_iterator it;
			for (it = bufs.begin(); it != bufs.end(); it++) {
				output_stream_->write((*it).buf, (*it).len);
				if (output_stream_->bad()) {
					ec = make_error_code(error::bad_stream);
					break ;
				}
			}
		} else if (vector_write_handler_) {
			ec = vector_write_handler_(connection_hdl_, bufs);
		} else if (write_handler_) {
			std::vector<buffer>::const_iterator it;
			for (it = bufs.begin(); it != bufs.end(); it++) {
				ec = write_handler_(connection_hdl_, (*it).buf, (*it).len);
				if (ec) {break ;}
			}
		} else {
			ec = make_error_code(error::output_stream_required);
		}

		handler(ec);
	}

	void set_handle(connection_hdl hdl) {
		connection_hdl_ = hdl;
	}

	std::error_code dispatch(zl_device_agent::dispatch_handler handler) {
		handler();
		return std::error_code();
	}

	// perform cleanup on socket shutdown_handler
	void async_shutdown(zl_device_agent::shutdown_handler handler) {
		std::error_code ec;
		if (shutdown_handler_) {
			ec = shutdown_handler_(connection_hdl_);
		}

		handler(ec);
	}

private:
	void read(std::istream& in) {
		std::cout << "iostream_con read" << std::endl;
		while (in.good()) {
			if (!reading_) {
				std::cerr << "write while not reading" << std::endl;
				break;
			}

			in.read(buf_ + cursor_,
				static_cast<std::streamsize>(len_ - cursor_));

			if (in.gcount() == 0) {
				std::cerr << "read zero bytes" << std::endl;
				break ;
			}

			cursor_ += static_cast<size_t>(in.gcount());

			// TODO error handling
			if (in.bad()) {
				reading_ = false;
				complete_read(make_error_code(error::bad_stream));
			}

			if (cursor_ >= bytes_needed_) {
				reading_ = false;
				complete_read(std::error_code());
			}
		}
	}

	size_t read_some_impl(char const* buf, size_t len) {
		std::cout << "iostream_con read_some" << std::endl;

		if (!reading_) {
			std::cerr << "write while not reading" << std::endl;
			return 0;
		}

		size_t bytes_to_copy = (std::min)(len, len_ -  cursor_);
		std::copy(buf, buf + bytes_to_copy, buf_ + cursor_);
		cursor_ += bytes_to_copy;

		if (cursor_ >= bytes_needed_) {
			complete_read(std::error_code());
		}

		return bytes_to_copy;
	}

	// signal that a requested read is complete
	void complete_read(std::error_code const& ec) {
		reading_ = false;
		read_handler handler = read_handler_;
		read_handler_ = read_handler();
		handler(ec, cursor_);
	}

private:
	bool is_server_;
	mutex_type read_mutex_;
	std::ostream* output_stream_;
	bool reading_;
	std::string remote_endpoint_;
	connection_hdl connection_hdl_;
	write_handler write_handler_;
	vector_write_handler vector_write_handler_;
	shutdown_handler shutdown_handler_;
	char* buf_;
	size_t len_;
	size_t bytes_needed_;
	read_handler read_handler_;
	size_t cursor_;
};
}	// iostream
}	// zl_device_agent

// test case

#include <mutex>

namespace zl_device_agent {
namespace concurrency {
class basic {
public:
	typedef std::mutex mutex_type;
	typedef std::lock_guard<mutex_type> scoped_lock_type;
};
}	// namespace concurrency
}	// namespace zl_device_agent

struct config {
	typedef zl_device_agent::concurrency::basic concurrency_type;
};

typedef zl_device_agent::iostream::connection<config> iostream_con;

struct stub_con : public iostream_con {
	typedef stub_con type;
	typedef std::shared_ptr<type> ptr;
	typedef iostream_con::timer_ptr timer_ptr;

	stub_con(bool is_server)
		: iostream_con(is_server)
		, ec(zl_device_agent::error::make_error_code(zl_device_agent::error::general))
		, indef_read_total(0)
	{}

	ptr get_shared() {
		return std::static_pointer_cast<type>(iostream_con::get_shared());
	}

	void write(std::string msg) {
		iostream_con::async_write(
			msg.data(),
			msg.size(),
			std::bind(
				&stub_con::handle_op,
				type::get_shared(),
				std::placeholders::_1
			)
		);
	}

	void write(std::vector<zl_device_agent::buffer>& bufs) {
		iostream_con::async_write(
			bufs,
			std::bind(
				&stub_con::handle_op,
				type::get_shared(),
				std::placeholders::_1
			)
		);
	}

	void async_read_at_least(size_t num_bytes, char* buf, size_t len) {
		iostream_con::async_read_at_least(
			num_bytes,
			buf,
			len,
			std::bind(
				&stub_con::handle_op,
				type::get_shared(),
				std::placeholders::_1
			)
		);
	}

	void handle_op(std::error_code const& e) {
		ec = e;
	}

	void async_read_indef(size_t num_bytes, char* buf, size_t len) {
		indef_read_size = num_bytes;
		indef_read_buf = buf;
		indef_read_len = len;

		indef_read();
	}

	void indef_read() {
		iostream_con::async_read_at_least(
			indef_read_size,
			indef_read_buf,
			indef_read_len,
			std::bind(
				&stub_con::handle_indef,
				type::get_shared(),
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
	}

	void handle_indef(std::error_code const& e, size_t amt_read) {
		ec = e;
		indef_read_total += amt_read;
		indef_read();
	}

	void shutdown() {
		iostream_con::async_shutdown(
			std::bind(
				&stub_con::handle_async_shutdown,
				type::get_shared(),
				std::placeholders::_1
			)
		);
	}

	void handle_async_shutdown(std::error_code const& e) {
		ec = e;
	}

	std::error_code ec;
	size_t indef_read_size;
	size_t indef_read_len;
	size_t indef_read_total;
	char* indef_read_buf;
};

// stubs
std::error_code write_handler(std::string& o, zl_device_agent::connection_hdl,
	char const* buf, size_t len) {
	o += std::string(buf, len);
	return std::error_code();
}

std::error_code vector_write_handler(std::string& o, zl_device_agent::connection_hdl,
	std::vector<zl_device_agent::buffer> const& bufs) {
	std::vector<zl_device_agent::buffer>::const_iterator it;
	for (it = bufs.begin(); it != bufs.end(); it++) {
		o += std::string((*it).buf, (*it).len);
	}

	return std::error_code();
}

std::error_code write_handler_error(zl_device_agent::connection_hdl,
	char const*, size_t) {
	return make_error_code(zl_device_agent::error::general);
}

/*
int main()
{
	return 0;
}
*/
