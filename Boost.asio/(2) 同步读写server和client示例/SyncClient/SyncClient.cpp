#include <boost/asio.hpp>
#include <iostream>
using namespace std;
using namespace boost::asio::ip;
const int MAX_LENGTH = 1024; // ���ͺͽ��յ���󳤶�

int main() {
	try {
		// ���������ķ���
		boost::asio::io_context ioc;
		// ���� endpoint
		tcp::endpoint remote_ep(make_address("127.0.0.1"), 10086);
		// ����socket
		tcp::socket sock(ioc);
		boost::system::error_code error = boost::asio::error::host_not_found;
		// ���ӶԶ�
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
		size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply, request_length)); // �����ٶ�����
		std::cout << "Reply is: ";
		std::cout.write(reply, reply_length); // ���յ����������������̨
		std::cout << "\n";
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}