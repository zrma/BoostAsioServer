//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "stdafx.h"
#include "HttpServer.hpp"
#include <signal.h>
#include <utility>

namespace Http
{
	namespace Server
	{
		Server::Server(const std::string& address, const std::string& port,
			const std::string& doc_root)
			: m_IOService(), m_Signals(m_IOService),
			m_Acceptor(m_IOService), m_Socket(m_IOService),
			m_ConnectionManager(), m_RequestHandler(doc_root)
		{
			// Register to handle the signals that indicate when the server should exit.
			// It is safe to register for the same signal multiple times in a program,
			// provided all registration for the specified signal is made through Asio.
			m_Signals.add(SIGINT);
			m_Signals.add(SIGTERM);

			#if defined(SIGQUIT)
			{
				m_Signals.add(SIGQUIT);
			}
			#endif // defined(SIGQUIT)

			// async_wait 를 미리 걸어둔다 - 시그널 대기용도
			DoAwaitStop();

			// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
			boost::asio::ip::tcp::resolver resolver(m_IOService);
			boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({ address, port });
			m_Acceptor.open(endpoint.protocol());
			m_Acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
			m_Acceptor.bind(endpoint);
			m_Acceptor.listen();

			DoAccept();
		}

		void Server::Run()
		{
			// The io_service::run() call will block until all asynchronous operations
			// have finished. While the server is running, there is always at least one
			// asynchronous operation outstanding: the asynchronous accept call waiting
			// for new incoming connections.
			m_IOService.run();
		}

		void Server::DoAccept()
		{
			// Accept 콜백
			m_Acceptor.async_accept(m_Socket,
				[this](boost::system::error_code ec)
			{
				// Check whether the server was stopped by a signal before this
				// completion handler had a chance to run.
				if (!m_Acceptor.is_open())
				{
					return;
				}

				if (!ec)
				{
					m_ConnectionManager.Start(std::make_shared<Connection>(
						std::move(m_Socket), m_ConnectionManager, m_RequestHandler));
				}

				DoAccept();
			});
		}

		void Server::DoAwaitStop()
		{
			// 비동기로 시그널에 대해 콜백 걸어둔다 - 콜백 호출되면 Acceptor와 ConnectionManager 닫고 정리
			m_Signals.async_wait(
				[this](boost::system::error_code /*ec*/, int /*signo*/)
			{
				// The server is stopped by cancelling all outstanding asynchronous
				// operations. Once all operations have finished the io_service::run()
				// call will exit.
				m_Acceptor.close();
				m_ConnectionManager.StopAll();
			});
		}

	} // namespace server
} // namespace http