#pragma once

#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

namespace BlockingTcpEchoServer
{
	const int max_length = 1024;
	bool stopFlag = false;

	void session(tcp::socket sock)
	{
		try
		{
			while ( !stopFlag )
			{
				char data[max_length];

				boost::system::error_code error;
				size_t length = sock.read_some(boost::asio::buffer(data), error);

				if (error == boost::asio::error::eof)
				{
					break; // Connection closed cleanly by peer.
				}
				else if (error)
				{
					throw boost::system::system_error(error); // Some other error.
				}

				// 읽은 것 그대로 쓰기(echo)
				boost::asio::write(sock, boost::asio::buffer(data, length));

				if (!strncmp(data, "exit", 4))
				{
					stopFlag = true;
					break;
				}
			}
		}
		catch (std::exception& e)
		{
			std::cerr << "Exception in thread: " << e.what() << "\n";
		}
	}

	void server(boost::asio::io_service& io_service, unsigned short port)
	{
		tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
		for (;;)
		{
			tcp::socket sock(io_service);
			a.accept(sock);

			if (stopFlag)
			{
				break;
			}
			std::thread(session, std::move(sock)).detach();
		}
	}
}