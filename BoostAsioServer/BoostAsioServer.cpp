// BoostAsioServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#include "Server.hpp"
#include "ReferenceCountedServer.hpp"
#include "ChatClient.hpp"
#include "ChatServer.hpp"

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>

const char* ip = "127.0.0.1";
const char* port = "9999";
const char* port1 = "9001";
const char* port2 = "9002";

void ServerTest()
{
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

void ReferenceCountedServerTest()
{
	try
	{
		boost::asio::io_service io_service;
		ReferenceCountedServer s(io_service, std::atoi(port));
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

void ChatTest()
{
	// 서버 스레드를 클라이언트 스레드와 분리
	std::thread serverThread = std::thread([&]()
	{
		// 서버 사이드
		try
		{
			boost::asio::io_service io_service;
			std::list<chat_server> servers;

			const char* ports[] = { port, port1, port2 };
			size_t portCount = sizeof(ports) / sizeof(char*);

			for (int i = 0; i < portCount; ++i)
			{
				tcp::endpoint endpoint(tcp::v4(), std::atoi( ports[i] ));
				servers.emplace_back(io_service, endpoint);
			}

			io_service.run();
		}
		catch (std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << "\n";
		}
	});

	Sleep(5000);

	// 클라이언트 사이드
	try
	{
		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		auto endpoint_iterator = resolver.resolve({ ip, port });
		chat_client c(io_service, endpoint_iterator);

		// 클라이언트 통신은 별개 스레드로 진행
		std::thread t([&io_service]() { io_service.run(); });

		char line[chat_message::max_body_length + 1];
		while (std::cin.getline(line, chat_message::max_body_length + 1))
		{
			chat_message msg;
			msg.body_length(std::strlen(line));
			std::memcpy(msg.body(), line, msg.body_length());
			msg.encode_header();
			c.write(msg);
		}

		c.close();
		t.join();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

int main()
{
	// ServerTest();
	// ReferenceCountedServerTest();

	ChatTest();

	return 0;
}