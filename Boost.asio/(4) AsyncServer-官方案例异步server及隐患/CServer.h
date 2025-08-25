#pragma once
#include <boost/asio.hpp>
#include "CSession.h"
#include <map>
#include <memory>
using boost::asio::ip::tcp;
using namespace std;

class CServer {
public:
	CServer(boost::asio::io_context& io_context, short port);
	void ClearSession(std::string uuid);
private:
	void StartAccept();
	void HandleAccept(shared_ptr<CSession> new_session, const boost::system::error_code& error);
	boost::asio::io_context& _io_context;
	short _port;
	tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
};
