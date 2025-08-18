#include <boost/asio.hpp>
#include <iostream>
using namespace std;
using namespace boost::asio::ip;
const int MAX_LENGTH = 1024; // 发送和接收的最大长度

int main() {
	try {
		// 创建上下文服务
		boost::asio::io_context ioc;
		// 构造 endpoint
		tcp::endpoint remote_ep(make_address("127.0.0.1"), 10086);
		// 创建socket
		tcp::socket sock(ioc);
		boost::system::error_code error = boost::asio::error::host_not_found;
		// 连接对端
		sock.connect(remote_ep, error);
		if (error) {
			std::cout << "Connect failed, code is " << error.value() <<
				".Message is " << error.message() << "\n";
			return 0;
		}

		std::cout << "Enter message: " << "\n";
		char request[MAX_LENGTH];
		std::cin.getline(request, MAX_LENGTH);
		size_t request_length = strlen(request);
		boost::asio::write(sock, boost::asio::buffer(request, request_length));

		char reply[MAX_LENGTH];
		size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply, request_length)); // 发多少读多少
		std::cout << "Reply is: ";
		std::cout.write(reply, reply_length); // 接收到的数据输出到控制台
		std::cout << "\n";
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}