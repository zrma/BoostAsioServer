#pragma once

#pragma warning ( disable : 4996 )

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

using boost::asio::ip::tcp;

// A reference-counted non-modifiable buffer class.
class shared_const_buffer
{
public:
	// Construct from a std::string.
	explicit shared_const_buffer(const std::string& data)
		: data_(new std::vector<char>(data.begin(), data.end())),
		buffer_(boost::asio::buffer(*data_))
	{
	}

	// Implement the ConstBufferSequence requirements.
	typedef boost::asio::const_buffer value_type;
	typedef const boost::asio::const_buffer* const_iterator;
	const boost::asio::const_buffer* begin() const { return &buffer_; }
	const boost::asio::const_buffer* end() const { return &buffer_ + 1; }

private:
	std::shared_ptr<std::vector<char> > data_;
	boost::asio::const_buffer buffer_;
};

class RefereceCountedSession
	: public std::enable_shared_from_this<RefereceCountedSession>
{
public:
	RefereceCountedSession(tcp::socket socket)
		: socket_(std::move(socket))
	{
	}

	void start()
	{
		do_write();
	}

private:
	void do_write()
	{
		std::time_t now = std::time(0);
		shared_const_buffer buffer(std::ctime(&now));

		auto self(shared_from_this());
		boost::asio::async_write(socket_, buffer,
			[this, self](boost::system::error_code /*ec*/, std::size_t /*length*/)
		{
		});
	}

	// The socket used to communicate with the client.
	tcp::socket socket_;
};

class ReferenceCountedServer
{
public:
	ReferenceCountedServer(boost::asio::io_service& io_service, short port)
		: acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
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
				std::make_shared<RefereceCountedSession>(std::move(socket_))->start();
			}

			do_accept();
		});
	}

	tcp::acceptor acceptor_;
	tcp::socket socket_;
};
