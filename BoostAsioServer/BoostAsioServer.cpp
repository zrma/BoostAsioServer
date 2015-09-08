// BoostAsioServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#pragma warning ( disable : 4005 )

#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// Class to manage the memory to be used for handler-based custom allocation.
// It contains a single block of memory which may be returned for allocation
// requests. If the memory is in use when an allocation request is made, the
// allocator delegates allocation to the global heap.
class handler_allocator
{
public:
	handler_allocator()
		: in_use_(false)
	{
	}

	// 명시적으로 복사 생성자 / 대입 연산자 disable 처리
	handler_allocator(const handler_allocator&) = delete;
	handler_allocator& operator=(const handler_allocator&) = delete;

	// 할당
	void* allocate(std::size_t size)
	{
		// 커스텀 메모리 사이즈 내에서 호출하는 것이면 바로 제공
		if (!in_use_ && size < sizeof(storage_))
		{
			in_use_ = true;
			return &storage_;
		}
		// 아니면 새로 할당
		else
		{
			return ::operator new(size);
		}
	}

	// 할당해제
	void deallocate(void* pointer)
	{
		// 커스텀 메모리 반납이면 flag만 false로
		if (pointer == &storage_)
		{
			in_use_ = false;
		}
		// 아니면 삭제
		else
		{
			::operator delete(pointer);
		}
	}

private:
	// Storage space used for handler-based custom memory allocation.
	std::aligned_storage<1024>::type storage_;

	// Whether the handler-based custom allocation storage has been used.
	bool in_use_;
};

// Wrapper class template for handler objects to allow handler memory
// allocation to be customised. Calls to operator() are forwarded to the
// encapsulated handler.
template <typename Handler>
class custom_alloc_handler
{
public:
	custom_alloc_handler(handler_allocator& a, Handler h)
		: allocator_(a), handler_(h)
	{
	}

	// 파라메터 바인딩
	template <typename ...Args>
	void operator()(Args&&... args)
	{
		handler_(std::forward<Args>(args)...);
	}

	// Asio allocator에 이 래핑클래스(의 멤버 handler_allocator) 등록
	friend void* asio_handler_allocate(std::size_t size,
		custom_alloc_handler<Handler>* this_handler)
	{
		return this_handler->allocator_.allocate(size);
	}

	// Asio deallocator에 이 래핑클래스(의 멤버 handler_allocator) 등록
	friend void asio_handler_deallocate(void* pointer, std::size_t /*size*/,
		custom_alloc_handler<Handler>* this_handler)
	{
		this_handler->allocator_.deallocate(pointer);
	}

private:
	handler_allocator& allocator_;
	Handler handler_;
};

// Helper function to wrap a handler object to add custom allocation.
// 중간 함수
template <typename Handler>
inline custom_alloc_handler<Handler> make_custom_alloc_handler(
	handler_allocator& a, Handler h)
{
	return custom_alloc_handler<Handler>(a, h);
}

//////////////////////////////////////////////////////////////////////////
// 중간함수 -> 래핑클래스 리턴
//
// 래핑 클래스 -> Asio alloc / dealloc 중개 해줌
//
// alloc / dealloc -> handler_allocator 가 aligend_storage로 컨트롤 or 동적 할당
//////////////////////////////////////////////////////////////////////////


class session
	: public std::enable_shared_from_this<session>
{
public:
	session(tcp::socket socket)
		// 소유권 이전(메모리 이동)
		: socket_(std::move(socket))
	{
	}

	void start()
	{
		do_read();
	}

private:
	void do_read()
	{
		// weak_ptr
		auto self(shared_from_this());

		socket_.async_read_some(
			// 버퍼
			boost::asio::buffer(data_),

			// Allocator
			make_custom_alloc_handler(allocator_,
				// Lambda 함수식 - 읽기 성공시 불려질 CallBack 함수
				[this, self](boost::system::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						do_write(length);
					}
				}));
	}

	// 에코
	void do_write(std::size_t length)
	{
		auto self(shared_from_this());
		boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
			make_custom_alloc_handler(allocator_,
				// Lambda 함수식 - 쓰기 성공하면 불려질 Callback 함수
				[this, self](boost::system::error_code ec, std::size_t /*length*/)
				{
					if (!ec)
					{
						do_read();
					}
				}));
	}

	// The socket used to communicate with the client.
	tcp::socket socket_;

	// Buffer used to store data received from the client.
	std::array<char, 1024> data_;

	// The allocator to use for handler-based custom memory allocation.
	handler_allocator allocator_;
};

class server
{
public:
	server(boost::asio::io_service& io_service, short port)
		: acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), socket_(io_service)
	{
		do_accept();
	}

private:
	void do_accept()
	{
		acceptor_.async_accept(socket_,
			// Lambda 함수식 - accept 성공하면 불려질 Callback 함수
			[this](boost::system::error_code ec)
			{
				if (!ec)
				{
					// 정상적으로 accept 성공하면 session 생성해서 Start() { read -> write -> read -> write -> ... }
					std::make_shared<session>(std::move(socket_))->start();
				}

				do_accept();
			});
	}

	// listener
	tcp::acceptor acceptor_;

	// accepted socket -> session 으로 옮겨짐(std::move)
	tcp::socket socket_;
};

int main()
{
	const char* port = "9999";
	try
	{
		boost::asio::io_service io_service;
		server s(io_service, std::atoi(port));
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}