#include "endpoints.h"
#include <boost/asio.hpp>
#include <iostream>
using namespace boost;

int client_end_point() {
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 3333;
	boost::system::error_code ec;
	asio::ip::address ip_address = asio::ip::make_address(raw_ip_address, ec);
	if (ec.value() != 0) {
		std::cout << "Failed to parse the IP address.Error code = " << ec.value() << ".Message is " << ec.message();
		return ec.value();
	}
	std::cout << "Success to parse the IP address." << "\n";

	// 一个端节点由服务端的IP地址和端口号组成，客户端便能通过这个端节点进行连接
	asio::ip::tcp::endpoint ep(ip_address, port_num);
	std::cout << "The client endpoint was successfully created" << "\n";
	return 0;
}

int server_end_point()
{
	unsigned short port_num = 3333;
	asio::ip::address ip_address = asio::ip::address_v6::any();
	asio::ip::tcp::endpoint ep(ip_address, port_num);
	return 0;
}

int create_tcp_socket()
{
	asio::io_context ioc;
	asio::ip::tcp protocol = asio::ip::tcp::v4();
	asio::ip::tcp::socket sock(ioc);
	boost::system::error_code ec;
	sock.open(protocol, ec);
	if (ec.value() != 0) {
		std::cout << "Failed to open the socket!Error code = " 
			<< ec.value() << ".Message is " << ec.message();
		return ec.value();
	}
	return 0;
}

int create_acceptor_socket()
{
	asio::io_context ioc;
#if 0
	asio::ip::tcp::acceptor acceptor(ioc);
	asio::ip::tcp protocol = asio::ip::tcp::v4();
	boost::system::error_code ec;
	acceptor.open(protocol, ec);
	if (ec.value() != 0) {
		std::cout << "Failed to open the acceptor socket!Error code = "
			<< ec.value() << ".Message is " << ec.message();
		return ec.value();
	}
#endif
	// 新版方法
	asio::ip::tcp::acceptor a(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 3333));
	return 0;
}

int bind_acceptor_socket()
{
	unsigned short port_num = 3333;
	// 接受来自任何地址的连接
	asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
	asio::io_context ioc;
	asio::ip::tcp::acceptor acceptor(ioc, ep.protocol());
	boost::system::error_code ec;
	acceptor.bind(ep, ec);
	if (ec.value() != 0) {
		std::cout << "Failed to bind the acceptor socket!Error code = "
			<< ec.value() << ".Message is " << ec.message();
		return ec.value();
	}
	return 0;
}

int connect_to_end()
{
	// 假设我们知道服务端的地址
	std::string raw_ip_address = "192.168.1.124";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol());
		sock.connect(ep);
	}
	catch (system::system_error& e) {
		std::cout << "Error occurred! Error code = " << e.code() << ".Message is " << e.what();
	
		return e.code().value();
	}
	return 0;
}

int dns_connect_to_end()
{
	std::string host = "llfc.club";
	std::string port_num = "3333";
	asio::io_context ioc;
	asio::ip::tcp::resolver resolver(ioc);
	try {
		asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, port_num);
		asio::ip::tcp::socket sock(ioc);
		asio::connect(sock, endpoints);
	}
	catch (system::system_error& e) {
		std::cout << "Error occurred! Error code = " << e.code() 
			<< ".Message is " << e.what();
		return e.code().value();
	}
	return 0;
}

int accept_new_connection()
{
	const int BACKLOG_SIZE = 30;
	unsigned short port_num = 3333;
	asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
	asio::io_context ioc;
	try {
		asio::ip::tcp::acceptor acceptor(ioc, ep.protocol());
		acceptor.bind(ep);
		acceptor.listen(BACKLOG_SIZE);
		asio::ip::tcp::socket sock(ioc);
		acceptor.accept(sock);
	}
	catch (system::system_error& e) {
		std::cout << "Error occurred! Error code = " << e.code()
			<< ".Message is " << e.what();
		return e.code().value();
	}
	return 0;
}

void use_const_buffer()
{
	std::string buf = "Hello world!";
	asio::const_buffer asio_buf(buf.c_str(), buf.length());
	std::vector<asio::const_buffer> buffers_sequence;
	buffers_sequence.push_back(asio_buf);
}

void use_buffer_str()
{
	asio::const_buffer output_buf = asio::buffer("Hello world!");
}

void use_buffer_array()
{
	const size_t BUF_SIZE_BYTES = 20;
	std::unique_ptr<char[]> buf(new char[BUF_SIZE_BYTES]);
	auto input_buf = asio::buffer(static_cast<void*>(buf.get()), BUF_SIZE_BYTES);
}

void write_to_socket(asio::ip::tcp::socket& sock)
{
	std::string buf = "Hello world!";
	std::size_t total_bytes_written = 0;
	// 循环发送，write_some返回每次写入的字节数
	while (total_bytes_written != buf.length()) {
		total_bytes_written += sock.write_some(asio::buffer(buf.c_str() + total_bytes_written, 
			buf.length() - total_bytes_written));
	}
}

int send_data_by_write_some() {
	std::string raw_ip_address = "192.168.44.128";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol()); // 也可以只传ioc，协议默认
		sock.connect(ep);
		write_to_socket(sock);
	}
	catch (system::system_error& e) {
		std::cout << "Error occurred! Error code = " << e.code()
			<< ".Message is " << e.what();
		return e.code().value();
	}
	return 0;
}

int send_data_by_send() {
	std::string raw_ip_address = "192.168.44.128";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol()); // 也可以只传ioc，协议默认
		sock.connect(ep);
		std::string buf = "Hello world!";
		int send_length = sock.send(asio::buffer(buf.c_str(), buf.length()));
		if (send_length <= 0) {
			return 0;
		}
	}
	catch (system::system_error& e) {
		std::cout << "Error occurred! Error code = " << e.code()
			<< ".Message is " << e.what();
		return e.code().value();
	}
	return 0;
}

int send_data_by_write() {
	std::string raw_ip_address = "192.168.44.128";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol()); // 也可以只传ioc，协议默认
		sock.connect(ep);
		std::string buf = "Hello world!";
		int send_length = asio::write(sock, asio::buffer(buf.c_str(), buf.length()));
		if (send_length <= 0) {
			return 0;
		}
	}
	catch (system::system_error& e) {
		std::cout << "Error occurred! Error code = " << e.code()
			<< ".Message is " << e.what();
		return e.code().value();
	}
	return 0;
}

std::string read_from_socket(asio::ip::tcp::socket& sock) {
	const unsigned char MESSAGE_SIZE = 7;
	char buf[MESSAGE_SIZE];
	std::size_t total_bytes_read = 0;
	while (total_bytes_read != MESSAGE_SIZE) {
		total_bytes_read += sock.read_some(asio::buffer(buf + total_bytes_read, MESSAGE_SIZE - total_bytes_read));
	}

	return std::string(buf, total_bytes_read);
}

int read_data_by_read_some() {
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol());
		sock.connect(ep);
		read_from_socket(sock);
	}
	catch (system::system_error& e) {
		std::cout << "Error occurred! Error code = " << e.code()
			<< ".Message is " << e.what();
		return e.code().value();
	}
	return 0;
}

int read_data_by_receive() {
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol());
		sock.connect(ep);
		const unsigned char BUFF_SIZE = 7;
		char buffer_receive[BUFF_SIZE];
		int receive_length = sock.receive(buffer_receive, BUFF_SIZE);
		if (receive_length <= 0) {
			std::cout << "receive failed!" << std::endl;
			return 0;
		}
	}
	catch (system::system_error& e) {
		std::cout << "Error occurred! Error code = " << e.code()
			<< ".Message is " << e.what();
		return e.code().value();
	}
	return 0;
}

int read_data_by_read() {
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol());
		sock.connect(ep);
		const unsigned char BUFF_SIZE = 7;
		char buffer_receive[BUFF_SIZE];
		int receive_length = asio::read(sock, asio::buffer(buffer_receive, BUFF_SIZE));
		if (receive_length <= 0) {
			std::cout << "receive failed!" << std::endl;
			return 0;
		}
	}
	catch (system::system_error& e) {
		std::cout << "Error occurred! Error code = " << e.code()
			<< ".Message is " << e.what();
		return e.code().value();
	}
	return 0;
}