#pragma once

#include <array>
#include <future>
#include <iostream>
#include <thread>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/use_future.hpp>

using boost::asio::ip::udp;

namespace DaytimeClient
{
	void get_daytime(boost::asio::io_service& io_service, const char* hostname)
	{
		try
		{
			udp::resolver resolver(io_service);

			// std::future : non-copyable asynchronous return object
			//
			// std::async()는 전달 받은 기능을 해당 스레드에서 비동기로 실행시키려 한다.
			// 비동기의 '미래 결과'에 접근 할 수 있다(반환값 or 예외)
			//
			// async가 만든 결과에 접근, 기능이 실행 되는지 확인하고 싶다면 future 객체가 필요하다.
			std::future<udp::resolver::iterator> iter =
				resolver.async_resolve(
					{ udp::v4(), hostname, "9999" },		// const query
					boost::asio::use_future					// resolve handler
					);

			// The async_resolve operation above returns the endpoint iterator as a
			// future value that is not retrieved ...

			udp::socket socket(io_service, udp::v4());

			std::array<char, 1> send_buf = { { 0 } };
			std::future<std::size_t> send_length =
				socket.async_send_to(boost::asio::buffer(send_buf),
					*iter.get(), // ... until here. This call may block.
					boost::asio::use_future);

			// Do other things here while the send completes.
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			send_length.get(); // Blocks until the send is complete. Throws any errors.

			std::array<char, 128> recv_buf;
			udp::endpoint sender_endpoint;
			std::future<std::size_t> recv_length =
				socket.async_receive_from(
					boost::asio::buffer(recv_buf),
					sender_endpoint,
					boost::asio::use_future);

			// Do other things here while the receive completes.
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			std::cout.write(
				recv_buf.data(),
				recv_length.get()); // Blocks until receive is complete.
		}
		catch (std::system_error& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
}