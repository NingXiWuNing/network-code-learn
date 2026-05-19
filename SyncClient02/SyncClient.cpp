//同步客户端小结：
//创建 io_context → 创建 endpoint → 创建 socket → 连接 → 发送 → 接收 → 打印
//特点：所有操作都是阻塞的，代码简单直观，但一个客户端只能顺序地一问一答，无法同时收发。
#include<boost/asio.hpp>
#include <iostream>

#define HEAD_LENGTH 2
using namespace std;
using namespace boost::asio::ip;
const int MAX_LENGTH = 1024;

int main()
{
	try
	{
		while (true)
		{
			//定义上下文用来创建连接客户端的socket
			boost::asio::io_context ioc;
			//创建一个远程端点，客户端用来连接，服务端用来绑定
			boost::asio::ip::tcp::endpoint remote_ep(make_address("127.0.0.1"), 10086);
			//创建socket用来连接服务端
			boost::asio::ip::tcp::socket sock(ioc);
			sock.connect(remote_ep);
			//用户写入数据并且发送给服务端
			char request[MAX_LENGTH];
			//使用getline读取整行避免遇到空格停止
			cout << "input message:" << endl;
			std::cin.getline(request, MAX_LENGTH);
			size_t send_length = strlen(request);
			char send[MAX_LENGTH + 2] = { 0 };
			::memcpy(send, &send_length, HEAD_LENGTH);
			::memcpy(send + HEAD_LENGTH, request, send_length);
			boost::asio::write(sock, boost::asio::buffer(send,send_length + 2));
			//客服端接收服务端发送回来的数据

			char reply_head[HEAD_LENGTH];
			//read阻塞读取，必须读取request_length的长度才会停止
			boost::asio::read(sock, boost::asio::buffer(reply_head,HEAD_LENGTH));
			short reply_length = 0;
			memcpy(&reply_length, reply_head, HEAD_LENGTH);
			cout << "reply len is : " << reply_length << endl;
			char reply[MAX_LENGTH] = { 0 };
			boost::asio::read(sock, boost::asio::buffer(reply, reply_length));
			//打印服务端发送回来的数据
			cout << "reply is: ";
			cout.write(reply, reply_length);
			cout << endl;
		}

	}
	catch(std::exception & error)
	{
		cout << "exception is: " << error.what() << endl;
	}
}