#include "Session.h"
#include<iostream>

int main()
{
	try
	{
		boost::asio::io_context ioc;
		Server s(ioc, 10086);
		ioc.run();
	}
	catch (std::exception& e)
	{
		cout << "exception is : " << e.what() << endl;
	}
	return 0;
}