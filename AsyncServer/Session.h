#pragma once
#include<iostream>
#include "Server.h"
#include<memory>
#include<boost/asio.hpp>
#include<boost/uuid/uuid_io.hpp>
#include<queue>
#include<boost/uuid/uuid_generators.hpp>
#include<mutex>
using boost::asio::ip::tcp;
using namespace std;
const int max_size = 1024;

class MsgNode
{
	friend class Session;
public:
	MsgNode(char* msg, int max_len) :max_len(max_len)
	{
		_data = new char[max_len];
		memcpy(_data, msg, max_len);
	}
	~MsgNode()
	{
		delete[] _data;
	}
private:
	char* _data;//数据的首地址
	int cur_size;//已经处理的长度
	int max_len;//数据的总长度
};

class Session :public std::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_context& ioc, Server* server) :_socket(ioc), _server(server)
	{
		boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
		_uuid = boost::uuids::to_string(a_uuid);
	}

	tcp::socket& Socket()
	{
		return _socket;
	}

	void Start();

	void send(char *data,int max_len);

	string& GetUuid() { return _uuid; };
private:
	tcp::socket _socket;
	char _data[max_size];
	void HandleRead(const boost::system::error_code& error, size_t bytes_transfered,shared_ptr<Session> _self_shared);
	void HandleWrite(const boost::system::error_code& error, shared_ptr<Session> _self_shared);
	Server* _server;
	std::string _uuid;
	queue<shared_ptr<MsgNode>> _send_que;
	mutex _send_lock;
};


