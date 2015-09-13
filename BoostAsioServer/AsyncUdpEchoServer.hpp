#pragma once

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

namespace AsyncUdpEcho
{
	class server
	{
	public:
		server(boost::asio::io_service& io_service, short port)
			: socket_(io_service, udp::endpoint(udp::v4(), port))
		{
			do_receive();
		}

		void do_receive()
		{
			// UDP ���� RecvFrom() �� async_receive_from �Լ��� ���
			socket_.async_receive_from(
				boost::asio::buffer(data_, max_length), sender_endpoint_,
				[this](boost::system::error_code ec, std::size_t bytes_recvd)
			{
				// TCP���� ������
				// 0����Ʈ Recv�� ���ؼ� do_receive ����
				if (!ec && bytes_recvd > 0)
				{
					do_send(bytes_recvd);
				}
				else
				{
					do_receive();
				}
			});
		}

		void do_send(std::size_t length)
		{
			// UDP ��忡���� SendTo() �� async_send_to �Լ��� ���
			socket_.async_send_to(
				boost::asio::buffer(data_, length), sender_endpoint_,
				[this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/)
			{
				do_receive();
			});
		}

	private:
		udp::socket socket_;
		udp::endpoint sender_endpoint_;
		enum { max_length = 1024 };
		char data_[max_length];
	};
}