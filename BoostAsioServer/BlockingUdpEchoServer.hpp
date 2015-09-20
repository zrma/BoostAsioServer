#pragma once

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

namespace BlockingUdpEchoServer
{
	enum { max_length = 1024 };
	bool stopFlag = false;

	void server(boost::asio::io_service& io_service, unsigned short port)
	{
		udp::socket sock(io_service, udp::endpoint(udp::v4(), port));
		while (!stopFlag)
		{
			char data[max_length];
			udp::endpoint sender_endpoint;
			size_t length = sock.receive_from(
				boost::asio::buffer(data, max_length), sender_endpoint);
			sock.send_to(boost::asio::buffer(data, length), sender_endpoint);

			if (!strncmp(data, "exit", 4))
			{
				stopFlag = true;
				break;
			}
		}
	}
}