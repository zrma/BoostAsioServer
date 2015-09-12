#pragma once

#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "ChatMessage.hpp"

using boost::asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
public:
	chat_client(boost::asio::io_service& io_service,
		tcp::resolver::iterator endpoint_iterator)
		: io_service_(io_service),
		socket_(io_service)
	{
		do_connect(endpoint_iterator);
	}

	void write(const chat_message& msg)
	{
		io_service_.post(
			[this, msg]()
		{
			bool write_in_progress = !write_msgs_.empty();
			write_msgs_.push_back(msg);

			//////////////////////////////////////////////////////////////////////////
			// 메세지큐에 메세지가 있다면 write_in_progress는 true
			// 아직 보내고 있는 중이니 do_write를 수행하지 않는다.
			//
			// 메세지큐가 비어있다면 write_in_progress는 false
			// 방금 집어넣은 msg를 do_write 수행!
			//////////////////////////////////////////////////////////////////////////
			if (!write_in_progress)
			{
				do_write();
			}
		});
	}

	void close()
	{
		io_service_.post([this]() { socket_.close(); });
	}

private:
	// 생성자에서 접속 시도
	void do_connect(tcp::resolver::iterator endpoint_iterator)
	{
		boost::asio::async_connect(socket_, endpoint_iterator,
			// 접속 완료 콜백 람다
			[this](boost::system::error_code ec, tcp::resolver::iterator)
		{
			if (!ec)
			{
				// 접속 성공하면 헤더 읽기
				do_read_header();
			}
		});
	}

	void do_read_header()
	{
		boost::asio::async_read(socket_,
			// 전체 데이터 중, 헤더 만큼 읽어와서 chat_message 버퍼에 담기
			boost::asio::buffer(read_msg_.data(), chat_message::header_length),
			// 읽기 완료 콜백 람다
			[this](boost::system::error_code ec, std::size_t /*length*/)
		{
			// 에러 코드 체크 및 페이로드 사이즈 검증
			if (!ec && read_msg_.decode_header())
			{
				// 페이로드 읽기
				do_read_body();
			}
			else
			{
				// 문제 생기면 무조건 자름
				socket_.close();
			}
		});
	}

	void do_read_body()
	{
		boost::asio::async_read(socket_,
			// 전체 데이터 중, 페이로드 사이즈 만큼 읽어와서 chat_message 버퍼에 담기
			boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
			// 읽기 완료 콜백 람다
			[this](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				// 읽어온 메세지 스탠다드 콘솔 출력
				std::cout.write(read_msg_.body(), read_msg_.body_length());
				std::cout << "\n";

				// 출력 끝났으면 PreRecv
				do_read_header();
			}
			else
			{
				socket_.close();
			}
		});
	}

	void do_write()
	{
		boost::asio::async_write(socket_,
			// 큐 맨 앞쪽에 담긴 chat_message의 전체 버퍼 전달
			boost::asio::buffer(write_msgs_.front().data(),	write_msgs_.front().length()),
			// 쓰기 완료 콜백 람다
			[this](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				// 쓰기 완료시 보낸 것 버림
				write_msgs_.pop_front();

				// 큐에 내용물 아직 남아있으면 더 보내기
				if (!write_msgs_.empty())
				{
					do_write();
				}
			}
			else
			{
				socket_.close();
			}
		});
	}

private:
	boost::asio::io_service& io_service_;
	tcp::socket socket_;
	chat_message read_msg_;
	chat_message_queue write_msgs_;
};
