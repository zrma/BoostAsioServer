// BoostAsioServer.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

#include "Server.hpp"
#include "ReferenceCounted.hpp"
#include "ChatClient.hpp"
#include "ChatServer.hpp"
#include "AsyncTcpEchoServer.hpp"
#include "AsyncUdpEchoServer.hpp"

const char* ip = "127.0.0.1";
const char* port = "9999";
const char* port1 = "9001";
const char* port2 = "9002";

void AllocationTest()
{
	try
	{
		boost::asio::io_service io_service;
		Allocation::server s(io_service, std::atoi(port));
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

void ReferenceCountedTest()
{
	try
	{
		boost::asio::io_service io_service;
		ReferenceCounted::server s(io_service, std::atoi(port));
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

void ChatTest()
{
	// ���� �����带 Ŭ���̾�Ʈ ������� �и�
	std::thread serverThread = std::thread([&]()
	{
		// ���� ���̵�
		try
		{
			boost::asio::io_service io_service;
			std::list<Chat::chat_server> servers;

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

	// Ŭ���̾�Ʈ ���̵�
	try
	{
		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		auto endpoint_iterator = resolver.resolve({ ip, port });
		Chat::chat_client c(io_service, endpoint_iterator);

		// Ŭ���̾�Ʈ ����� ���� ������� ����
		std::thread t([&io_service]() { io_service.run(); });

		char line[Chat::chat_message::max_body_length + 1];
		while (std::cin.getline(line, Chat::chat_message::max_body_length + 1))
		{
			Chat::chat_message msg;
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

void AsyncTcpEchoTest()
{
	try
	{
		boost::asio::io_service io_service;

		AsyncTcpEcho::server s(io_service, std::atoi( port ));

		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

void AsyncUdpEchoTest()
{
	try
	{
		boost::asio::io_service io_service;

		AsyncUdpEcho::server s(io_service, std::atoi( port ));

		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

int main()
{
	// AllocationTest();
	// ReferenceCountedTest();
	// ChatTest();

	// AsyncTcpEchoTest();
	// AsyncUdpEchoTest();

	return 0;
}