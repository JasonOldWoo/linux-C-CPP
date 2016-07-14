#include <string>

class connection_metadata {
public:
	connection_metadata(int id, std::string uri)
		: id_(id), uri_(uri)
	{
	}

private:
	int id_;
	std::string uri_;
};
