#include "Session.h"

void Session::HandleWrite(const boost::system::error_code& error,shared_ptr<Session> _self_shared)
{
	if (!error)
	{
		lock_guard<mutex> lock(_send_lock);
		_send_que.pop();
		if (_send_que.size())
		{
			auto& msg = _send_que.front();
			boost::asio::async_write(_socket, boost::asio::buffer(msg->_data,msg->max_len),
				bind(&Session::HandleWrite, this, placeholders::_1, _self_shared));
		}
	}
	else
	{
		cout << "faild in HandleWrite error is: " << error.what() << endl;
		_server->ClearSession(_self_shared->GetUuid());
	}
}

void Session::HandleRead(const boost::system::error_code& error, size_t bytes_transfered, shared_ptr<Session> _self_shared)
{
	if (!error)
	{
		cout << "receive data is : " << _data<< endl;

		send(_data, bytes_transfered);

		memset(_data, 0, sizeof _data);

		_socket.async_read_some(boost::asio::buffer(_data, max_size),
			bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, _self_shared));
	}
	else
	{
		if(error != boost::asio::error::eof)
		cout << "faild in HandleRead error is: " << error.what() << endl;

		_server->ClearSession(_self_shared->GetUuid());
	}
}

void Session::Start()
{
	memset(_data, 0, sizeof _data);
	_socket.async_read_some(boost::asio::buffer(_data, max_size),
		bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2,shared_from_this()));
}

void Session::send(char* msg,int max_length)
{
	bool pending = false; // 判断发送队列是否为空
	lock_guard<mutex> lock(_send_lock);//增加一个锁防止多个线程乱码

	if (_send_que.size() > 0) pending = true;

	_send_que.push(make_shared<MsgNode>(msg, max_length));

	if (pending) return;

	boost::asio::async_write(_socket, boost::asio::buffer(msg, max_length),
		bind(&Session::HandleWrite, this, placeholders::_1, shared_from_this()));
}

