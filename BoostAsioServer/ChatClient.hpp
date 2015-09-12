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
			// �޼���ť�� �޼����� �ִٸ� write_in_progress�� true
			// ���� ������ �ִ� ���̴� do_write�� �������� �ʴ´�.
			//
			// �޼���ť�� ����ִٸ� write_in_progress�� false
			// ��� ������� msg�� do_write ����!
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
	// �����ڿ��� ���� �õ�
	void do_connect(tcp::resolver::iterator endpoint_iterator)
	{
		boost::asio::async_connect(socket_, endpoint_iterator,
			// ���� �Ϸ� �ݹ� ����
			[this](boost::system::error_code ec, tcp::resolver::iterator)
		{
			if (!ec)
			{
				// ���� �����ϸ� ��� �б�
				do_read_header();
			}
		});
	}

	void do_read_header()
	{
		boost::asio::async_read(socket_,
			// ��ü ������ ��, ��� ��ŭ �о�ͼ� chat_message ���ۿ� ���
			boost::asio::buffer(read_msg_.data(), chat_message::header_length),
			// �б� �Ϸ� �ݹ� ����
			[this](boost::system::error_code ec, std::size_t /*length*/)
		{
			// ���� �ڵ� üũ �� ���̷ε� ������ ����
			if (!ec && read_msg_.decode_header())
			{
				// ���̷ε� �б�
				do_read_body();
			}
			else
			{
				// ���� ����� ������ �ڸ�
				socket_.close();
			}
		});
	}

	void do_read_body()
	{
		boost::asio::async_read(socket_,
			// ��ü ������ ��, ���̷ε� ������ ��ŭ �о�ͼ� chat_message ���ۿ� ���
			boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
			// �б� �Ϸ� �ݹ� ����
			[this](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				// �о�� �޼��� ���Ĵٵ� �ܼ� ���
				std::cout.write(read_msg_.body(), read_msg_.body_length());
				std::cout << "\n";

				// ��� �������� PreRecv
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
			// ť �� ���ʿ� ��� chat_message�� ��ü ���� ����
			boost::asio::buffer(write_msgs_.front().data(),	write_msgs_.front().length()),
			// ���� �Ϸ� �ݹ� ����
			[this](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				// ���� �Ϸ�� ���� �� ����
				write_msgs_.pop_front();

				// ť�� ���빰 ���� ���������� �� ������
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
