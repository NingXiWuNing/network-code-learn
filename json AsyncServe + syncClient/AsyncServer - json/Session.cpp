#include "Session.h"

Session::Session(boost::asio::io_context& ioc, Server* server) :_socket(ioc), _server(server), b_head_parse(false),b_close(false)
{
	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
	_uuid = boost::uuids::to_string(a_uuid);
	rece_head_node = make_shared<MsgNode>(HEAD_LENGTH);
}

Session::~Session()
{
	std::cout << "~Session hapend" << endl;
}


void Session::Start()
{
	::memset(_data, 0, sizeof _data);
	_socket.async_read_some(boost::asio::buffer(_data, max_size),
		bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, shared_from_this()));
}

void Session::Close()
{
	_socket.close();
	b_close = true;
}

string& Session::GetUuid()
{
	return _uuid;
}

void Session::send(char* msg, int max_length)
{
	bool pending = false; // 判断发送队列是否为空
	lock_guard<mutex> lock(_send_lock);//增加一个锁防止多个线程乱码

	if (_send_que.size() > MAX_SENDQUE)
	{
		cout << "session : " << _uuid << "is fulled size is :" << MAX_SENDQUE << endl;
		return;
	}

	_send_que.push(make_shared<MsgNode>(msg, max_length));

	if (pending) return;
	
	auto& msg_node = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msg_node->_data, msg_node->total_len),
		bind(&Session::HandleWrite, this, placeholders::_1, shared_from_this()));
}


void Session::HandleWrite(const boost::system::error_code& error,shared_ptr<Session> _self_shared)
{
	if (!error)
	{
		lock_guard<mutex> lock(_send_lock);
		_send_que.pop();
		if (_send_que.size())
		{
			auto& msg = _send_que.front();
			boost::asio::async_write(_socket, boost::asio::buffer(msg->_data,msg->total_len),
				bind(&Session::HandleWrite, this, placeholders::_1, _self_shared));
		}
	}
	else
	{
		std::cout << "faild in HandleWrite error is: " << error.what() << endl;
		_server->ClearSession(_self_shared->GetUuid());
	}
}

void Session::HandleRead(const boost::system::error_code& error, size_t bytes_transfered, shared_ptr<Session> _self_shared)
{
	if (!error)
	{
		int copy_len = 0;
		while (bytes_transfered > 0)
		{
			if (!b_head_parse)
			{
				if (rece_head_node->cur_len + bytes_transfered < HEAD_LENGTH)
				{
					memcpy(rece_head_node->_data + rece_head_node->cur_len, _data + copy_len, bytes_transfered);
					rece_head_node->cur_len += bytes_transfered;
					memset(_data, 0, max_size);
					_socket.async_read_some(boost::asio::buffer(_data, max_size),
						bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, _self_shared));
					return;
				}

				int head_remain = HEAD_LENGTH - rece_head_node->cur_len;
				memcpy(rece_head_node->_data + rece_head_node->cur_len, _data + copy_len, head_remain);
				copy_len += head_remain;
				bytes_transfered -= head_remain;

				short data_len = 0;
				memcpy(&data_len, rece_head_node->_data, HEAD_LENGTH);

				//网络字节转换为本地字节序
				data_len = boost::asio::detail::socket_ops::network_to_host_short(data_len);

				std::cout << "data len is :" << data_len << endl;

				if (data_len > MAX_LENGTH)
				{
					std::cout << "Word data exceeds the maximum allowed length" << endl;
					_server->ClearSession(GetUuid());
					return;
				}

				rece_msg_node = make_shared<MsgNode>(data_len);
				//信息体的长度不足以满足头部大小,先存起来当前数据
				if (bytes_transfered < data_len)
				{
					memcpy(rece_msg_node->_data + rece_msg_node->cur_len, _data + copy_len, bytes_transfered);
					rece_msg_node->cur_len += bytes_transfered;
					::memset(_data, 0, max_size);
					_socket.async_read_some(boost::asio::buffer(_data, max_size),
						bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, _self_shared));

					b_head_parse = true;
					return;
				}

				memcpy(rece_msg_node->_data + rece_msg_node->cur_len, _data + copy_len, data_len);
				bytes_transfered -= data_len;
				copy_len += data_len;
				rece_msg_node->cur_len += data_len;
				rece_msg_node->_data[rece_msg_node->total_len] = '\0';

				Json::Value root;
				Json::Reader reader;
				reader.parse(std::string(rece_msg_node->_data, data_len), root);
				std::cout << "receive message id is : " <<root["id"] <<" receive message data is"<<root["data"] << endl;

				send(rece_msg_node->_data, rece_msg_node->total_len);

				b_head_parse = false;
				rece_head_node->Clear();

				if (bytes_transfered <= 0)
				{
					memset(_data, 0, max_size);
					_socket.async_read_some(boost::asio::buffer(_data, max_size),
						bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, _self_shared));
					return;
				}
				continue;
			}


			int msg_remain = rece_msg_node->total_len - rece_msg_node->cur_len;
			if (bytes_transfered < msg_remain)
			{
				memcpy(rece_msg_node->_data + rece_msg_node->cur_len, _data + copy_len, bytes_transfered);
				::memset(_data, 0, max_size);
				rece_msg_node->cur_len += bytes_transfered;
				_socket.async_read_some(boost::asio::buffer(_data, max_size),
					bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, _self_shared));
				return;
			}

			memcpy(rece_msg_node->_data + rece_msg_node->cur_len, _data + copy_len, msg_remain);

			rece_msg_node->_data[rece_msg_node->total_len] = '\0';

			Json::Value root;
			Json::Reader reader;
			reader.parse(std::string(rece_msg_node->_data,rece_msg_node->total_len), root);
			std::cout << "receive message id is : " << root["id"] << " receive message data is" << root["data"] << endl;

			send(rece_msg_node->_data, rece_msg_node->total_len);

			b_head_parse = false;
			copy_len += msg_remain;
			bytes_transfered -= msg_remain;
			rece_head_node->Clear();

			if (bytes_transfered <= 0)
			{
				memset(_data, 0, max_size);
				_socket.async_read_some(boost::asio::buffer(_data, max_size),
					bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, _self_shared));
				return;
			}
		}
	}
	else
	{
		if (boost::asio::error::eof != error)
		{
			std::cout << "HandleRead error is : " << error.what() << endl;
		}
		Close();
		_server->ClearSession(_uuid);
	}
	
}









