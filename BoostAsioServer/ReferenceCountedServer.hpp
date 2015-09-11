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
	// explicit - 명시적 생성자
	explicit shared_const_buffer(const std::string& data)
		// string 객체를 벡터 배열로 저장
		: data_(new std::vector<char>(data.begin(), data.end())),
		// 부스트 버퍼로 저장
		buffer_(boost::asio::buffer(*data_))
	{
	}

	//////////////////////////////////////////////////////////////////////////
	// 부스트 const buffer를 사용하기 위해 채워줘야 하는 것들
	//////////////////////////////////////////////////////////////////////////

	// Implement the ConstBufferSequence requirements.
	typedef boost::asio::const_buffer value_type;
	typedef const boost::asio::const_buffer* const_iterator;
	const boost::asio::const_buffer* begin() const { return &buffer_; }
	const boost::asio::const_buffer* end() const { return &buffer_ + 1; }
	//////////////////////////////////////////////////////////////////////////

private:
	std::shared_ptr<std::vector<char> > data_;
	boost::asio::const_buffer buffer_;
};

class RefereceCountedSession
	: public std::enable_shared_from_this<RefereceCountedSession>
{
public:
	RefereceCountedSession(tcp::socket socket)
		// 소유권 이전(메모리 이동)
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
		// 현재 시각
		std::time_t now = std::time(0);

		// 현재 시각을 출력 형식에 맞춰 버퍼에 담음
		shared_const_buffer buffer(std::ctime(&now));

		// weak_ptr
		auto self(shared_from_this());

		// 콜백 등록
		boost::asio::async_write(socket_, buffer,
			[this, self](boost::system::error_code /*ec*/, std::size_t /*length*/)
		{
			// 완료 되면 아무 것도 안 함
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
				// 정상적으로 접속하면 세션 start -> 버퍼에 현재 시각을 담아서 리턴 해 줌
				std::make_shared<RefereceCountedSession>(std::move(socket_))->start();
			}

			do_accept();
		});
	}

	tcp::acceptor acceptor_;
	tcp::socket socket_;
};
