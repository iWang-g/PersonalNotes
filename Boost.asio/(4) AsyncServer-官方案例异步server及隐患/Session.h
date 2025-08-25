#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <map>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <queue>
#include <memory>
#define MAX_LENGTH 1024*2
#define HEAD_LENGTH 2
using boost::asio::ip::tcp;
using namespace std;

class Server;

class MsgNode {
	friend class Session;
public:
	MsgNode(char* msg, short max_len) : _total_len(max_len + HEAD_LENGTH), _cur_len(0) {
		_data = new char[_total_len + 1]();
		memcpy(_data, &max_len, HEAD_LENGTH);
		memcpy(_data + HEAD_LENGTH, msg, max_len);
		_data[_total_len] = '\0';
	}
	
	MsgNode(short max_len) : _total_len(max_len), _cur_len(0) {
		_data = new char[_total_len + 1]();
	}

	~MsgNode() {
		delete[] _data;
	}

	void Clear() {
		::memset(_data, 0, _total_len);
		_cur_len = 0;
	}

private:
	short _cur_len;
	short _total_len;
	char* _data;
};

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_context& ioc, Server* server) 
		: _socket(ioc), _server(server), _b_close(false), _b_head_parse(false) {
		boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
		_uuid = boost::uuids::to_string(a_uuid);
		_recv_head_node = make_shared<MsgNode>(HEAD_LENGTH);
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
	void Close();
	std::shared_ptr<Session> SharedSelf();

private:
	void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred, shared_ptr<Session> _self_shared);
	void handle_write(const boost::system::error_code& error, shared_ptr<Session> _self_shared);
	void HandleRead(const boost::system::error_code& error, std::size_t bytes_transferred, shared_ptr<Session> _self_shared);
	void HandleWrite(const boost::system::error_code& error, shared_ptr<Session> _self_shared);

	tcp::socket _socket;
	char _data[MAX_LENGTH];
	Server* _server;
	std::string _uuid;
	std::queue<shared_ptr<MsgNode>> _send_que;
	std::mutex _send_lock;
	bool _b_close;
	std::shared_ptr<MsgNode> _recv_msg_node; // 收到的消息结构
	bool _b_head_parse; // 头部是否解析完成
	std::shared_ptr<MsgNode> _recv_head_node; // 收到的头部结构
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