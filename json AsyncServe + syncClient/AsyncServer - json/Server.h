#pragma once
#include<iostream>
#include<boost/asio.hpp>
#include<map>

using boost::asio::ip::tcp;
using namespace std;

class Session;

class Server
{
public:
	Server(boost::asio::io_context& ioc,unsigned short port_num);
	void ClearSession(std::string uuid);
private:
	boost::asio::io_context& _ioc;
	tcp::acceptor _acceptor;
	void StartAccept();
	void HandleAccept(shared_ptr<Session> session, const boost::system::error_code& error);
	std::map <string, shared_ptr<Session>> _sessions;
};


