#include "Session.h"

void Session::Start()
{
	memset(_data, 0, max_length);
	_socket.async_read_some(boost::asio::buffer(_data, max_length),
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
		memset(_data, 0, max_length);
		_socket.async_read_some(boost::asio::buffer(_data, max_length),
			std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
	}
	else {
		cout << "Write error, error value is " << error.value() << ".Message is " << error.message() << endl;
		// delete this;
		_server->ClearSession(_uuid);
	}
}

void Session::HandleRead(const boost::system::error_code& error, std::size_t bytes_transferred, shared_ptr<Session> _self_shared)
{
	if (!error) {
		cout << "Server receive data is: " << _data << endl;
		Send(_data, bytes_transferred);
		memset(_data, 0, max_length);
		_socket.async_read_some(boost::asio::buffer(_data, max_length),
			std::bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, _self_shared));
	}
	else {
		cout << "Handle read error, error value is " << error.value() << ".Message is " << error.message() << endl;
		_server->ClearSession(_uuid);
	}
}

void Session::HandleWrite(const boost::system::error_code& error, shared_ptr<Session> _self_shared)
{
	if (!error) {
		std::lock_guard<std::mutex> lock(_send_lock);
		_send_que.pop();
		if (!_send_que.empty()) {
			auto& msgnode = _send_que.front();
			boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_max_len),
				std::bind(&Session::HandleWrite, this, std::placeholders::_1, _self_shared));
		}
	}
	else {
		cout << "Handle write error, error value is " << error.value() << ".Message is " << error.message() << endl;
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