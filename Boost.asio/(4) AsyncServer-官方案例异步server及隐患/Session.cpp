#include "Session.h"

void Session::Start()
{
	memset(_data, 0, MAX_LENGTH);
	_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
		std::bind(&Session::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

std::string& Session::GetUuid()
{
	return _uuid;
}

void Session::Send(char* msg, int length)
{
	// pending为true表示发送队列里有数据，也就是上一次的数据未发完
	// pending为false表示上一次的数据发完了，发送缓冲区是空的
	bool pending = false;
	std::lock_guard<std::mutex> lock(_send_lock);
	if (_send_que.size() > 0) {
		pending = true;
	}
	_send_que.push(make_shared<MsgNode>(msg, length));
	if (pending) {
		return;
	}

	boost::asio::async_write(_socket, boost::asio::buffer(msg, length),
		std::bind(&Session::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}

void Session::Close()
{
	_socket.close();
	_b_close = true;
}

std::shared_ptr<Session> Session::SharedSelf()
{
	return shared_from_this();
}

void Session::handle_read(const boost::system::error_code& error, std::size_t bytes_transferred, shared_ptr<Session> _self_shared) {
	if (!error) {
		cout << "Server receive data is: " << _data << endl;
		
		boost::asio::async_write(_socket, boost::asio::buffer(_data, bytes_transferred), 
			std::bind(&Session::handle_write, this, std::placeholders::_1, _self_shared));
	}
	else {
		cout << "Read error, error value is " << error.value() << ".Message is " << error.message() << endl;
		// delete this;
		_server->ClearSession(_uuid);
	}
}

void Session::handle_write(const boost::system::error_code& error, shared_ptr<Session> _self_shared) {
	if (!error) {
		memset(_data, 0, MAX_LENGTH);
		_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
			std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
	}
	else {
		cout << "Write error, error value is " << error.value() << ".Message is " << error.message() << endl;
		// delete this;
		_server->ClearSession(_uuid);
	}
}

void Session::HandleRead(const boost::system::error_code& error, size_t  bytes_transferred, std::shared_ptr<Session> shared_self) {
	if (!error) {
		//已经移动的字符数
		int copy_len = 0;
		while (bytes_transferred > 0) {
			if (!_b_head_parse) {
				//收到的数据不足头部大小
				if (bytes_transferred + _recv_head_node->_cur_len < HEAD_LENGTH) {
					memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
					_recv_head_node->_cur_len += bytes_transferred;
					::memset(_data, 0, MAX_LENGTH);
					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
						std::bind(&Session::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
					return;
				}
				//收到的数据比头部多
				//头部剩余未复制的长度
				int head_remain = HEAD_LENGTH - _recv_head_node->_cur_len;
				memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
				//更新已处理的data长度和剩余未处理的长度
				copy_len += head_remain;
				bytes_transferred -= head_remain;
				//获取头部数据
				short data_len = 0;
				memcpy(&data_len, _recv_head_node->_data, HEAD_LENGTH);
				cout << "data_len is " << data_len << endl;
				//头部长度非法
				if (data_len > MAX_LENGTH) {
					std::cout << "invalid data length is " << data_len << endl;
					_server->ClearSession(_uuid);
					return;
				}
				_recv_msg_node = make_shared<MsgNode>(data_len);
				//消息的长度小于头部规定的长度，说明数据未收全，则先将部分消息放到接收节点里
				if (bytes_transferred < data_len) {
					memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
					_recv_msg_node->_cur_len += bytes_transferred;
					::memset(_data, 0, MAX_LENGTH);
					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
						std::bind(&Session::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
					//头部处理完成
					_b_head_parse = true;
					return;
				}
				memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, data_len);
				_recv_msg_node->_cur_len += data_len;
				copy_len += data_len;
				bytes_transferred -= data_len;
				_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
				cout << "receive data is " << _recv_msg_node->_data << endl;
				//此处可以调用Send发送测试
				Send(_recv_msg_node->_data, _recv_msg_node->_total_len);
				//继续轮询剩余未处理数据
				_b_head_parse = false;
				_recv_head_node->Clear();
				if (bytes_transferred <= 0) {
					::memset(_data, 0, MAX_LENGTH);
					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
						std::bind(&Session::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
					return;
				}
				continue;
			}
			//已经处理完头部，处理上次未接受完的消息数据
			//接收的数据仍不足剩余未处理的
			int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
			if (bytes_transferred < remain_msg) {
				memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
				_recv_msg_node->_cur_len += bytes_transferred;
				::memset(_data, 0, MAX_LENGTH);
				_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
					std::bind(&Session::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
				return;
			}
			memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remain_msg);
			_recv_msg_node->_cur_len += remain_msg;
			bytes_transferred -= remain_msg;
			copy_len += remain_msg;
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
			cout << "receive data is " << _recv_msg_node->_data << endl;
			//此处可以调用Send发送测试
			Send(_recv_msg_node->_data, _recv_msg_node->_total_len);
			//继续轮询剩余未处理数据
			_b_head_parse = false;
			_recv_head_node->Clear();
			if (bytes_transferred <= 0) {
				::memset(_data, 0, MAX_LENGTH);
				_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
					std::bind(&Session::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
				return;
			}
			continue;
		}
	}
	else {
		std::cout << "handle read failed, error is " << error.what() << endl;
		Close();
		_server->ClearSession(_uuid);
	}
}

void Session::HandleWrite(const boost::system::error_code& error, shared_ptr<Session> _self_shared)
{
	if (!error) {
		std::lock_guard<std::mutex> lock(_send_lock);
		cout << "send data " << _send_que.front()->_data + HEAD_LENGTH << endl;
		_send_que.pop();
		if (!_send_que.empty()) {
			auto& msgnode = _send_que.front();
			boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
				std::bind(&Session::HandleWrite, this, std::placeholders::_1, _self_shared));
		}
	}
	else {
		cout << "Handle write error, error value is " << error.value() << ".Message is " << error.message() << endl;
		Close();
		_server->ClearSession(_uuid);
	}
}

Server::Server(boost::asio::io_context& ioc, short port) 
	: _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{
	cout << "Server start success, on port: " << port << endl;
	start_accept();
}

void Server::ClearSession(std::string uuid)
{
	_sessions.erase(uuid);
}

void Server::start_accept()
{
	shared_ptr<Session> new_session = make_shared<Session>(_ioc, this);
	_acceptor.async_accept(new_session->Socket(), std::bind(&Server::handle_accept, this, new_session, std::placeholders::_1));
}

void Server::handle_accept(shared_ptr<Session> new_session, const boost::system::error_code& error)
{
	if (!error) {
		new_session->Start();
		_sessions.insert(make_pair(new_session->GetUuid(), new_session));
	}
	else {
		// delete new_session;
	}
	start_accept();
}