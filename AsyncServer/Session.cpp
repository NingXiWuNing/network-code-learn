#include "Session.h"

void Session::handle_write(const boost::system::error_code& error)
{
	if (!error)
	{
		memset(data, '\0', sizeof data);
		_socket.async_read_some(boost::asio::buffer(data, max_size),
			bind(&Session::handle_read, this, placeholders::_1,placeholders::_2));
	}
	else
	{
		delete this;
	}
}

void Session::handle_read(const boost::system::error_code& error, size_t bytes_transfered)
{
	if (!error)
	{
		cout << "receive data is : ";
		cout.write(data, bytes_transfered);
		cout << endl;
		boost::asio::async_write(_socket, boost::asio::buffer(data, bytes_transfered),
			bind(&Session::handle_write, this, placeholders::_1));
	}
	else
	{
		delete this;
	}
}

void Session::Start()
{
	memset(data, '\0', sizeof data);
	_socket.async_read_some(boost::asio::buffer(data, max_size),
		bind(&Session::handle_read, this, placeholders::_1, placeholders::_2));
}

void Server::handle_accept(Session* session,const boost::system::error_code& error)
{
	if (!error)
	{
		session->Start();
	}
	else
	{
		delete session;
	}

	start_accept();
}

void Server::start_accept()
{
	Session* session = new Session(_ioc);
	_acceptor.async_accept(session->Socket(),
		bind(&Server::handle_accept,this,session,placeholders::_1));
}

Server::Server(boost::asio::io_context& ioc, unsigned short port_num)
	:_ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port_num))
{
	start_accept();
}