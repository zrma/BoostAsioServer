// BoostAsioServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#include "Server.hpp"
#include "ReferenceCounted.hpp"

#include "ChatClient.hpp"
#include "ChatServer.hpp"

#include "AsyncTcpEchoServer.hpp"
#include "AsyncUdpEchoServer.hpp"
#include "BlockingTcpEchoServer.hpp"
#include "BlockingUdpEchoServer.hpp"

#include "DaytimeClient.hpp"

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
	// 서버 스레드를 클라이언트 스레드와 분리
	std::thread serverThread = std::thread([&]()
	{
		// 서버 사이드
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

	// 클라이언트 사이드
	try
	{
		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		auto endpoint_iterator = resolver.resolve({ ip, port });
		Chat::chat_client c(io_service, endpoint_iterator);

		// 클라이언트 통신은 별개 스레드로 진행
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

enum { max_length = 1024 };

void BlockingTcpEchoTest()
{
	// 서버 스레드를 클라이언트 스레드와 분리
	std::thread serverThread = std::thread([&]()
	{
		// 서버 사이드
		try
		{
			boost::asio::io_service io_service;

			BlockingTcpEchoServer::server(io_service, std::atoi(port));
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
		for (;;)
		{
			boost::asio::io_service io_service;

			tcp::socket s(io_service);
			tcp::resolver resolver(io_service);
			boost::asio::connect(s, resolver.resolve({ ip, port }));

			// 서버가 accept를 중단했으면 클라이언트 루프 중지
			//
			// 루프 중지 하기 직전에 connect 하는 이유는 서버가 accept 단에서 block 상태로 대기 중인데, 
			// 서버의 block 상태를 풀어줌으로써 서버 스레드를 진행(후 종료)시키기 위해서
			if (BlockingTcpEchoServer::stopFlag)
			{
				break;
			}

			std::cout << "Enter message: ";
			char request[max_length] = { 0, };
			std::cin.getline(request, max_length);
			size_t request_length = std::strlen(request);
			boost::asio::write(s, boost::asio::buffer(request, request_length));

			char reply[max_length];
			size_t reply_length = boost::asio::read(s, boost::asio::buffer(reply, request_length));
			std::cout << "Reply is: ";
			std::cout.write(reply, reply_length);
			std::cout << "\n";
		}		
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	// 서버 스레드 종료 대기
	serverThread.join();
}

void BlockingUdpEchoTest()
{
	// 서버 스레드를 클라이언트 스레드와 분리
	std::thread serverThread = std::thread([&]()
	{
		// 서버 사이드
		try
		{
			boost::asio::io_service io_service;

			BlockingUdpEchoServer::server(io_service, std::atoi(port));
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

		udp::socket s(io_service, udp::endpoint(udp::v4(), 0));

		udp::resolver resolver(io_service);
		udp::endpoint endpoint = *resolver.resolve({ udp::v4(), ip, port });

		while (!BlockingUdpEchoServer::stopFlag)
		{
			std::cout << "Enter message: ";
			char request[max_length];
			std::cin.getline(request, max_length);
			size_t request_length = std::strlen(request);
			s.send_to(boost::asio::buffer(request, request_length), endpoint);

			char reply[max_length];
			udp::endpoint sender_endpoint;
			size_t reply_length = s.receive_from(boost::asio::buffer(reply, max_length), sender_endpoint);
			std::cout << "Reply is: ";
			std::cout.write(reply, reply_length);
			std::cout << "\n";
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	// 서버 스레드 종료 대기
	serverThread.join();
}

void DaytimeClientTest()
{
	boost::asio::io_service io_service_for_server;
	boost::asio::io_service::work work_for_server(io_service_for_server);

	// 서버 스레드를 클라이언트 스레드와 분리
	std::thread serverThread = std::thread([&]()
	{
		try
		{
			AsyncUdpEcho::server s(io_service_for_server, std::atoi(port));

			io_service_for_server.run();
		}
		catch (std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << "\n";
		}
	});

	try
	{		
		// We run the io_service off in its own thread so that it operates
		// completely asynchronously with respect to the rest of the program.
		boost::asio::io_service io_service;
		boost::asio::io_service::work work(io_service);
		std::thread thread([&io_service]() { io_service.run(); });

		DaytimeClient::get_daytime(io_service, ip);

		io_service.stop();
		thread.join();

		io_service_for_server.stop();
		serverThread.join();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

int main()
{
	// AllocationTest();
	// ReferenceCountedTest();
	// ChatTest();

	// AsyncTcpEchoTest();
	// AsyncUdpEchoTest();
	// BlockingTcpEchoTest();
	// BlockingUdpEchoTest();
	DaytimeClientTest();

	return 0;
}