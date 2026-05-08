#include<boost/asio.hpp>
#include <iostream>
#include<set>
#include<memory>
#include<thread>
#include<cstring>

const int max_length = 1024;
using boost::asio::ip::tcp;
using namespace std;
//一个智能指针类型，指向 tcp::socket
typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;
set<std::shared_ptr<std::thread>> thread_set; //临时存子线程的socket指针，避免循环结束socket指针被析构

//处理单个客户端的连接 ,循环读数据 → 打印 → 原样写回
void session(socket_ptr sock)
{
	try
	{
		while(true)
		{
			//创建一个data用来存接收到的数据
			//必须先初始化大小为read_some建立缓冲区
			char data[max_length];
			memset(data, '\0', max_length);
			//用read_some()函数之前需要先创建一个buffer和错误码
			boost::system::error_code error;
			//阻塞读取max_length数据长度用data来存，返回实际读取的长度
			size_t data_length = sock->read_some(boost::asio::buffer(data,max_length), error);
			//如果错误码是boost::asio中正常断开的错误退出循环即可
			if (boost::asio::error::eof == error)
			{
				std::cout << "connect close by peer"<< endl;
				break;
			}
			else if (error)
			{
				throw boost::system::system_error(error);
			}

			//打印接收到信息的客户端的ip地址
			cout << "receive data from : " << sock->remote_endpoint().address().to_string() << endl;
			//打印接收到的数据
			cout << "receive message is: ";
			cout.write(data, data_length);
			cout << endl;
			//回传相同的信息给客户端
			boost::asio::write(*sock, boost::asio::buffer(data,data_length));
		}
	}
	catch (std::exception& error)
	{
		cout << "exception is " << error.what() << endl;
	}
}

//服务端接收新的连接，并创建新的线程 ，无限循环接受连接 → 创建 socket → 创建线程 → 启动 session
void server(boost::asio::io_context& ioc, unsigned short port)   //boost::asio::io_context 不可拷贝只能按址传参
{
	//服务端的任意端点都能接收来自客户端的连接请求
	tcp::acceptor a(ioc, tcp::endpoint(tcp::v4(), port));

	while(true)
	{
		//创建一个空的socket用来存接收到的连接
		socket_ptr sock(new tcp::socket(ioc));
		//阻塞等待新的连接到来,接收到了就传给空的socket建立与客户端的连接
		a.accept(*sock);
		//创建一个线程专门用来服务这个客户端
		auto t = std::make_shared<std::thread>(session, sock);
		//把这个线程加入到集合中，便于后面主线程的结束
		thread_set.insert(t);
	}
}

int main()
{
	try
	{
		//创建ioc
		boost::asio::io_context ioc;
		//调用server
		server(ioc, 10086);
		//
		for (auto& t : thread_set)
		{
			t->join();
		}
	}
	catch (std:: exception & error)
	{
		cout << "exception is " << error.what() << endl;
	}

	return 0;
}
