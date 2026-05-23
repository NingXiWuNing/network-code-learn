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
#define HEAD_LENGTH 2
#define	MAX_LENGTH 1024 * 2

class MsgNode
{
	friend class Session;
public:
	MsgNode(char* msg, short max_len) :total_len(max_len + HEAD_LENGTH),cur_len(0)
	{
		_data = new char[total_len + 1];
		memcpy(_data, &max_len, HEAD_LENGTH);
		memcpy(_data + HEAD_LENGTH,msg, max_len);
	}

	MsgNode(short max_len):cur_len(0),total_len(max_len)
	{
		_data = new char[max_len + 1];
	}
	~MsgNode()
	{
		delete[] _data;
	}

	void Clear()
	{
		::memset(_data, 0, total_len);
		cur_len = 0;
	}
private:
	char* _data;//数据的首地址
	int cur_len;//已经处理的长度
	int total_len;//数据的总长度
};

class Session :public std::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_context& ioc, Server* server);

	tcp::socket& Socket()
	{
		return _socket;
	}

	~Session();

	void Start();

	void Close();

	void send(char* data, int max_len);

	void PrintRecvData(char* data, int length);

	string& GetUuid();
private:
	tcp::socket _socket;
	char _data[max_size];
	void HandleHeadRead(const boost::system::error_code& error, size_t bytes_transfered, shared_ptr<Session> _self_shared);
	void HandleMsgRead(const boost::system::error_code& error, size_t bytes_transfered, shared_ptr<Session> _self_shared);
	void HandleRead(const boost::system::error_code& error, size_t bytes_transfered, shared_ptr<Session> _self_shared);
	void HandleWrite(const boost::system::error_code& error, shared_ptr<Session> _self_shared);
	Server* _server;
	std::string _uuid;
	queue<shared_ptr<MsgNode>> _send_que;
	mutex _send_lock;
	bool b_close;
	//接收到数据的头部信息
	shared_ptr<MsgNode> rece_head_node;
	//头部信息是否解析完
	bool b_head_parse;
	//接收信息的数据信息
	shared_ptr<MsgNode> rece_msg_node;

};


