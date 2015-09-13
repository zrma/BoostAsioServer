#pragma once

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include "ChatMessage.hpp"

using boost::asio::ip::tcp;

namespace Chat
{
	typedef std::deque<chat_message> chat_message_queue;

	// 추상 클래스(인터페이스)
	class chat_participant
	{
	public:
		virtual ~chat_participant() {}
		virtual void deliver(const chat_message& msg) = 0;
	};

	typedef std::shared_ptr<chat_participant> chat_participant_ptr;

	class chat_room
	{
	public:
		void join(chat_participant_ptr participant)
		{
			// 셋 삽입
			participants_.insert(participant);

			// 새로 들어온 세션에 최근 채팅 보여주기
			// ranged for
			for (auto msg : recent_msgs_)
			{
				participant->deliver(msg);
			}
		}

		void leave(chat_participant_ptr participant)
		{
			participants_.erase(participant);
		}

		void deliver(const chat_message& msg)
		{
			recent_msgs_.push_back(msg);

			while (recent_msgs_.size() > max_recent_msgs)
			{
				recent_msgs_.pop_front();
			}

			for (auto participant : participants_)
			{
				participant->deliver(msg);
			}
		}

	private:
		// 세션 set
		std::set<chat_participant_ptr> participants_;
		enum { max_recent_msgs = 100 };
		chat_message_queue recent_msgs_;
	};

	//----------------------------------------------------------------------

	class chat_session
		: public chat_participant,
		public std::enable_shared_from_this<chat_session>
	{
	public:
		chat_session(tcp::socket socket, chat_room& room)
			: socket_(std::move(socket)),
			room_(room)
		{
		}

		void start()
		{
			// weaked_ptr로 방에 입장시키고
			room_.join(shared_from_this());

			// 헤더 읽기 시작
			do_read_header();
		}

		void deliver(const chat_message& msg)
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
		}

	private:
		void do_read_header()
		{
			auto self(shared_from_this());

			boost::asio::async_read(socket_,
				// 리드 버퍼 전달해서 헤더만 채워 오기
				boost::asio::buffer(read_msg_.data(), chat_message::header_length),
				[this, self](boost::system::error_code ec, std::size_t /*length*/)
			{
				// 오류 검사 & 헤더 검증
				if (!ec && read_msg_.decode_header())
				{
					// 성공하면 몸통 읽기
					do_read_body();
				}
				else
				{
					room_.leave(shared_from_this());
				}
			});
		}

		void do_read_body()
		{
			auto self(shared_from_this());
			boost::asio::async_read(socket_,
				// 리드 버퍼 전달해서 몸통 채워오기
				boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
				[this, self](boost::system::error_code ec, std::size_t /*length*/)
			{
				if (!ec)
				{
					// 채팅방에 메세지 뿌리기
					room_.deliver(read_msg_);

					// 다시 헤더만 읽어오기 PreRecv()
					do_read_header();
				}
				else
				{
					room_.leave(shared_from_this());
				}
			});
		}

		void do_write()
		{
			auto self(shared_from_this());
			boost::asio::async_write(socket_,
				// 쓰기 버퍼 전달
				boost::asio::buffer(write_msgs_.front().data(),
					write_msgs_.front().length()),
				[this, self](boost::system::error_code ec, std::size_t /*length*/)
			{
				if (!ec)
				{
					// 쓰기 완료 되었으므로 큐에서 지우고
					write_msgs_.pop_front();

					// 큐에 내용이 더 있으면
					if (!write_msgs_.empty())
					{
						// 또 쓰기(Send)
						do_write();
					}
				}
				else
				{
					room_.leave(shared_from_this());
				}
			});
		}

		tcp::socket socket_;
		chat_room& room_;
		chat_message read_msg_;
		chat_message_queue write_msgs_;
	};

	//----------------------------------------------------------------------

	class chat_server
	{
	public:
		chat_server(boost::asio::io_service& io_service,
			const tcp::endpoint& endpoint)
			: acceptor_(io_service, endpoint),
			socket_(io_service)
		{
			do_accept();
		}

	private:
		void do_accept()
		{
			acceptor_.async_accept(socket_,
				[this](boost::system::error_code ec)
			{
				if (!ec)
				{
					std::make_shared<chat_session>(std::move(socket_), room_)->start();
				}

				do_accept();
			});
		}

		tcp::acceptor acceptor_;
		tcp::socket socket_;
		chat_room room_;
	};
}