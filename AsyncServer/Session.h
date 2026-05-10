#pragma once
#include<iostream>
#include<boost/asio.hpp>

using boost::asio::ip::tcp;
using namespace std;
const int max_size = 1024;

class Session
{
public:
	Session(boost::asio::io_context& ioc) :_socket(ioc) {};

	tcp::socket& Socket()
	{
		return _socket;
	}

	void Start();
private:
	tcp::socket _socket;
	char data[max_size];
	void handle_read(const boost::system::error_code& error, size_t bytes_transfered);
	void handle_write(const boost::system::error_code& error);
};

class Server
{
public:
	Server(boost::asio::io_context& ioc,unsigned short port_num);
private:
	boost::asio::io_context& _ioc;
	tcp::acceptor _acceptor;
	void start_accept();
	void handle_accept(Session* session, const boost::system::error_code& error);
};

