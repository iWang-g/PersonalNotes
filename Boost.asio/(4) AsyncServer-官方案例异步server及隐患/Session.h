#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <map>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <queue>
#include <memory>
using boost::asio::ip::tcp;
using namespace std;

class Server;

class MsgNode {
	friend class Session;
public:
	MsgNode(char* msg, int max_len) {
		_data = new char[max_len];
		memcpy(_data, msg, max_len);
	}
	
	~MsgNode() {
		delete[] _data;
	}

private:
	int _cur;
	int _max_len;
	char* _data;
};

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_context& ioc, Server* server) : _socket(ioc), _server(server) {
		boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
		_uuid = boost::uuids::to_string(a_uuid);
	}

	tcp::socket& Socket() {
		return _socket;
	}

	~Session() {
		cout << "Session destruct delete this" << endl;
	}

	void Start();
	std::string& GetUuid();
	void Send(char* msg, int length);

private:
	void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred, shared_ptr<Session> _self_shared);
	void handle_write(const boost::system::error_code& error, shared_ptr<Session> _self_shared);
	void HandleRead(const boost::system::error_code& error, std::size_t bytes_transferred, shared_ptr<Session> _self_shared);
	void HandleWrite(const boost::system::error_code& error, shared_ptr<Session> _self_shared);

	tcp::socket _socket;
	enum { max_length = 1024 };
	char _data[max_length];
	Server* _server;
	std::string _uuid;
	std::queue<shared_ptr<MsgNode>> _send_que;
	std::mutex _send_lock;
};

class Server {
public:
	Server(boost::asio::io_context& ioc, short port);
	void ClearSession(std::string uuid);
private:
	void start_accept();
	void handle_accept(shared_ptr<Session> new_session, const boost::system::error_code& error);
	boost::asio::io_context& _ioc;
	tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<Session>> _sessions;
};