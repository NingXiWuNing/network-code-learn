//同步客户端小结：
//创建 io_context → 创建 endpoint → 创建 socket → 连接 → 发送 → 接收 → 打印
//特点：所有操作都是阻塞的，代码简单直观，但一个客户端只能顺序地一问一答，无法同时收发。
#include<boost/asio.hpp>
#include <iostream>
#include<json/json.h>
#include<json/reader.h>
#include<json/value.h>

#define HEAD_LENGTH 2
using namespace std;
using namespace boost::asio::ip;
const int MAX_LENGTH = 1024;

int main()
{
	try
	{
		for(int i=1;i<=10;i++)
		{
			//定义上下文用来创建连接客户端的socket
			boost::asio::io_context ioc;
			//创建一个远程端点，客户端用来连接，服务端用来绑定
			boost::asio::ip::tcp::endpoint remote_ep(make_address("127.0.0.1"), 10086);
			//创建socket用来连接服务端
			boost::asio::ip::tcp::socket sock(ioc);
			sock.connect(remote_ep);

			//利用json序列化发送一个复杂的类对象
			Json::Value root;
			root["id"] = 1001;
			root["data"] = "hello wrold";
			std::string request = root.toStyledString();

			short send_length = request.size();

			cout << send_length << endl;
			char send[MAX_LENGTH + 2] = { 0 };
			short send_net_length = boost::asio::detail::socket_ops::host_to_network_short(send_length);
			::memcpy(send, &send_net_length, HEAD_LENGTH);
			::memcpy(send + HEAD_LENGTH, request.c_str(), send_length);
			boost::asio::write(sock, boost::asio::buffer(send,send_length + 2));
			//客服端接收服务端发送回来的数据

			char reply_head[HEAD_LENGTH];
			//read阻塞读取，必须读取request_length的长度才会停止
			boost::asio::read(sock, boost::asio::buffer(reply_head,HEAD_LENGTH));
			short reply_length = 0;
			memcpy(&reply_length, reply_head, HEAD_LENGTH);
			short reply_host_length = boost::asio::detail::socket_ops::network_to_host_short(reply_length);
			cout << "reply len is : " << reply_host_length << endl;

			char reply[MAX_LENGTH] = { 0 };
			size_t reply_size = boost::asio::read(sock, boost::asio::buffer(reply, reply_host_length));
			//打印服务端发送回来的数据
			//定义json reader 对象解析发送回来的对象 并将其放进root中
			Json::Reader reader;
			reader.parse(std::string(reply, reply_size), root);
			cout << "reply id is : " << root["id"] << " reply data is : " << root["data"] << endl;
		}

	}
	catch(std::exception & error)
	{
		cout << "exception is: " << error.what() << endl;
	}
}