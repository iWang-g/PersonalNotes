#pragma once
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <queue>
#include <memory>
#define MAX_LENGTH 1024*2
#define HEAD_LENGTH 2
using boost::asio::ip::tcp;
using namespace std;

class CServer;

class MsgNode {
	friend class CSession;
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

class CSession : public std::enable_shared_from_this<CSession>
{
public:
	CSession(boost::asio::io_context& io_context, CServer* server);
	~CSession();
	tcp::socket& GetSocket();
	std::string& GetUuid();
	void Start();
	void Send(char* msg, int length);
	void Close();
	std::shared_ptr<CSession> SharedSelf();

private:
	void HandleRead(const boost::system::error_code& error, std::size_t bytes_transferred, shared_ptr<CSession> shared_self);
	void HandleWrite(const boost::system::error_code& error, shared_ptr<CSession> shared_self);
	tcp::socket _socket;
	char _data[MAX_LENGTH];
	CServer* _server;
	std::string _uuid;
	std::queue<shared_ptr<MsgNode>> _send_que;
	std::mutex _send_lock;
	bool _b_close;
	std::shared_ptr<MsgNode> _recv_msg_node; // 收到的消息结构
	bool _b_head_parse; // 头部是否解析完成
	std::shared_ptr<MsgNode> _recv_head_node; // 收到的头部结构
};