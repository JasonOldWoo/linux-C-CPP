#include "../socket_op_decl.hpp"

using zl_device_agent::cbuffer;

void print(cbuffer* buf) {
	for (cbuffer* item = buf->get_head(); 
		item && item->get_next() != item->get_head();
		item = item->get_next()) {
		std::cout << "head: " << item->get_head() << std::endl
			<< "end: " << item->get_end() << std::endl
			<< "len: " << item->get_len() << std::endl
			<< "total: " << item->get_total() << std::endl
			<< "trans: " << item->get_trans() << std::endl
			<< "data: " << item->get_data() << std::endl
			<< "consumed: " << item->get_consumed() << std::endl
			<< "cur: " << item->get_cur() << std::endl
			<< "self: " << item << std::endl;
		std::cout << std::endl;
	}
}

int main() {
	char msg1[10] = "msg1";
	std::size_t len1 = 10;
	char msg2[11] = "msg2";
	std::size_t len2 = 11;
	cbuffer buf1(msg1, len1);
	cbuffer buf2(msg2, len2);
	buf1.append(&buf2);
	print(&buf1);

	std::cout << "try making loop\n" << std::endl;
	buf2.append(&buf1);
	print(&buf1);

	char msg3[12] = "msg3";
	std::size_t len3 = 12;
	cbuffer buf3(msg3, len3);

	std::cout << "apend 1 2 to 3\n" << std::endl;
	buf3.append(&buf1);
	print(&buf1);

	std::cout << "try cutting\n" << std::endl;
	buf3.append(&buf2);
	print(&buf1);

	std::cout << "add bytes\n" << std::endl;
	buf3.add_bytes(20);
	print(&buf1);

	std::cout << "split\n" << std::endl;
	cbuffer* b1 = 0;
	cbuffer* b2 = 0;
	buf3.split_from_cur(b1, b2);
	std::cout << "b1:" << std::endl;
	print(b1);
	std::cout << "b2" << std::endl;
	print(b2);

	char msg4[15] = "msg4";
	std::size_t len4 = 15;
	cbuffer buf4(msg4, len4);
	return 0;
}
