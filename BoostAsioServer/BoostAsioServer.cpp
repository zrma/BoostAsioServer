// BoostAsioServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#include "Server.hpp"
#include "ReferenceCountedServer.hpp"

void ServerTest()
{
	const char* port = "9999";
	try
	{
		boost::asio::io_service io_service;
		server s(io_service, std::atoi(port));
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

int main()
{
	ServerTest();

	return 0;
}