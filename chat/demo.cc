#include <iostream>

class wja {
public:
	wja(int a)
		: data_(a)
	{}

	int data_;
};

inline wja get_service() {
	return wja(1);
}

int main() {
	int i = 0;
	switch (i) {
	// jesus, wtf?
	case 1:
	for (;;) {
		std::cout << "1" << std::endl;
		return 1; default:
		std::cout << "else" << std::endl;
		break;
	}
	}
	wja w = get_service();
	std::cout << w.data_ << std::endl;
	return 0;
}
