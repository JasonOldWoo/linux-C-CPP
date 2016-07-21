#ifndef __DEVICEAGENT_SOCKET_OP_DECL_HPP__
#define __DEVICEAGENT_SOCKET_OP_DECL_HPP__
#include "../../../config/posix.h"
#include "../../async/proactor.hpp"
#include "../../async/operation.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

namespace zl_device_agent {

typedef async_io::proactor<core_config> core_proactor;
typedef core_proactor* core_proactor_ptr;

struct cbuffer {
	cbuffer()

	{
		construct();
	}

	cbuffer(char* data, std::size_t data_len)
	{
		construct(data, data_len);
	}

	bool set_data(char* data, std::size_t data_len) {
		if (consumed) {
			return false;
		}

		head->bytes_total = head->bytes_total + data_len - len;
		msg = data;
		len = data_len;

		return true;
	}

	// 合并队列
	bool append(cbuffer* buf) {
		// 1.当前必须为队尾
		// 2.新队列没有消耗
		// 3.必须是队首数据
		//
		// 其中1,2虽然不是必要,但是可以使流程更清晰
		// 这里保证了任何两个队列都不包含相同元素
		if (buf == head || next
			|| buf->head->bytes_trans || buf->prev) {
			return false;
		}

		next = buf;
		if (buf) {
			buf->prev = this;
			head->bytes_total += buf->head->bytes_total;
			head->count += buf->head->count;
			head->end = buf->head->end;
		}
		// actually no loop
		for (cbuffer* item = head;
				item && item->next != head;
				item = item->next) {
			item->head = head;
		}
		return true;
	}

	char* get_data() {
		return msg;
	}

	std::size_t get_len() {
		return len;
	}

	// 获取传输长度
	std::size_t get_trans() {
		return head->bytes_trans;
	}

	std::size_t get_total() {
		return head->bytes_total;
	}

	// 获取消息个数
	std::size_t get_size() {
		return head->count;
	}

	// 获取当前消息的消耗数量
	std::size_t get_consumed() {
		return consumed;
	}

	cbuffer* get_next() {
		return next;
	}

	cbuffer* get_prev() {
		return prev;
	}

	cbuffer* get_head() {
		return head;
	}

	cbuffer* get_end() {
		return head->end;
	}

	// 获取待发送位置的指针
	cbuffer* get_cur() {
		return head->cur;
	}

	void reset() {
		cbuffer* item = head;
		for (; item; ) {
			cbuffer* tmp = item->next;
			item->construct();
			item = tmp;
		}
	}

	void unlink() {
		cbuffer* item = head;
		for (; item; ) {
			cbuffer* tmp = item->next;
			item->construct(item->msg, item->len);
			item = tmp;
		}
	}

	// 从指针位置进行分割
	void split_from_cur(cbuffer*& buf1, cbuffer*& buf2) {
		if (cur && cur->prev) {
			cbuffer* tmp_head = head;
			buf1 = tmp_head;
			buf2 = cur;
			cbuffer* item = 0;
			tmp_head->bytes_total = 0;
			tmp_head->count = 0;
			for (item = tmp_head; item && item != cur;
				item = item->next) {
				tmp_head->bytes_total += item->len;
				tmp_head->end = item;
				tmp_head->count++;
			}
			tmp_head->bytes_trans = tmp_head->bytes_total;

			cur->prev->next = 0;
			cur->prev = 0;
			cur->bytes_total = 0;
			cur->bytes_trans = cur->consumed;
			cur->count = 0;
			for (item = cur; item; item = item->next) {
				item->head = cur;
				item->head->bytes_total += item->len;
				cur->count++;
			}
			tmp_head->cur = 0;
		} else {
			buf1 = 0;
			buf2 = cur;
			return ;
		}
	}
	
	// 消耗数量累加,并且更新指针位置
	// 当指针为0时表示数据被消耗完毕
	void add_bytes(std::size_t bytes) {
#if 0
		head->bytes_trans += bytes;
		cbuffer* item = 0;
		for (item = head; item && bytes; item = item->next) {
			if (item->consumed < item->len) {
				std::size_t remain = item->len - item->consumed;
				int cmp = remain - bytes;
				item->consumed = (cmp > 0) ?
					(item->consumed + bytes) : (item->len);
				if (cmp > 0) {
					head->cur = item;
					return ;
				} else if (0 == cmp) {
					head->cur = item->next;
					return ;
 				}	else {
					bytes -= remain;
				}
			}
		}
		// maybe null
		head->cur = item;
#else
		head->bytes_trans += bytes;
		if (head->bytes_trans > head->bytes_total) {
			head->bytes_trans = head->bytes_total;
		}

		set_bytes(head->bytes_trans);
#endif
	}

	void set_bytes(std::size_t bytes) {
		head->bytes_trans = bytes > head->bytes_total
			? head->bytes_total : bytes;
		cbuffer* item = 0;
		for (item = head; item; item = item->next) {
			item->consumed = 0;
		}
		for (item = head; item; item = item->next) {
			if (item->len < bytes) {
				item->consumed = len;
				bytes -= len;
			} else if (item->len > bytes) {
				item->consumed = bytes;
				bytes = 0;
				head->cur = item;
				return ;
			} else {
				item->consumed = bytes;
				bytes = 0;
				head->cur = item->next;
				return ;
			}
		}
	}

private:
	void construct(char* data = 0, std::size_t data_len = 0) {
		msg = data;
		len = data_len;
		consumed = 0;
		bytes_trans = 0;
		bytes_total = data_len;
		count = 1;
		next = 0;
		prev = 0;
		head = this;
		end = this;
		cur = this;
	}

private:
	char* msg;
	std::size_t len;
	std::size_t consumed;
	std::size_t bytes_trans;
	std::size_t bytes_total;
	std::size_t count;

	cbuffer* next;
	cbuffer* prev;
	cbuffer* head;
	cbuffer* end;
	cbuffer* cur;
};

static void destroy_buffer_list(cbuffer* buf) {
	cbuffer* item = buf->get_head();
	for (; item; ) {
		cbuffer* tmp = item->get_next();
		delete item->get_data();
		item = tmp;
	}

	buf->reset();
}

class socket_write_op;
class socket_read_op;
}	// namespace zl_device_agent
#endif	// __deviceegent_socket_op_decl_hpp__
