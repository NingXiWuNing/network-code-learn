#include "Server.h"
#include "Session.h"
#include<boost/uuid/uuid_io.hpp>
#include<boost/uuid/uuid_generators.hpp>

void Server::HandleAccept(shared_ptr<Session> session, const boost::system::error_code& error)
{
	if (!error)
	{ 
		session->Start();
		_sessions.insert(make_pair(session->GetUuid(), session));
	}
	else
	{
		cout << "faild in HandleAccept error is: " << error.what() << endl;
	}

	StartAccept();
}

void Server::StartAccept()
{
	shared_ptr<Session> session = make_shared<Session>(_ioc,this);
	_acceptor.async_accept(session->Socket(),
		bind(&Server::HandleAccept, this, session, placeholders::_1));
}

Server::Server(boost::asio::io_context& ioc, unsigned short port_num)
	:_ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port_num))
{
	StartAccept();
}

void Server::ClearSession(string uuid)
{
	_sessions.erase(uuid);
}