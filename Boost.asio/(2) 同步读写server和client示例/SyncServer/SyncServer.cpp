// SyncServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <boost/asio.hpp>
#include <set>
#include <memory>
using namespace std;
using boost::asio::ip::tcp;
const int MAX_LENGTH = 1024;
typedef std::shared_ptr<tcp::socket> socket_ptr;
std::set<std::shared_ptr<std::thread>> thread_set;
using namespace std;

void session(socket_ptr sock) {
	try {
		for (;;) {
			char data[MAX_LENGTH];
			memset(data, '\0', MAX_LENGTH);
			boost::system::error_code error;
			// size_t length = boost::asio::read(sock, boost::asio::buffer(data, MAX_LENGTH), error);
			size_t length = sock->read_some(boost::asio::buffer(data, MAX_LENGTH), error);
			if (error == boost::asio::error::eof) { // eof 表示对端关闭
				std::cout << "Connection closed by peer" << "\n";
				break;
			}
			else if (error) {
				throw boost::system::system_error(error);
			}

			cout << "Receive from " << sock->remote_endpoint().address().to_string() << "\n";
			cout << "Receive message is " << data << "\n";
			// 把收到的数据回传给对方
			boost::asio::write(*sock, boost::asio::buffer(data, length));
		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

void server(boost::asio::io_context& io_context, unsigned short port) {
	tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
	for (;;) {
		socket_ptr socket(new tcp::socket(io_context));
		a.accept(*socket);
		auto t = std::make_shared<std::thread>(session, socket);
		thread_set.insert(t);
	}
}

int main()
{
	try {
		boost::asio::io_context ioc;
		server(ioc, 10086);
		for (auto& t : thread_set) {
			t->join();
		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}