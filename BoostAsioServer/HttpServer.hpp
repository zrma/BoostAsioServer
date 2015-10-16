//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include "Connection.hpp"
#include "connectionManager.hpp"
#include "RequestHandler.hpp"

namespace Http
{
	namespace Server
	{
		/// The top-level class of the HTTP server.
		class Server
		{
		public:
			// 일반 대입 복사 생성자 명시적으로 삭제
			Server(const Server&) = delete;
			Server& operator=(const Server&) = delete;

			/// Construct the server to listen on the specified TCP address and port, and
			/// serve up files from the given directory.
			// IP + Port + 경로 데이터가 모두 명시적으로 필요함!
			explicit Server(const std::string& address, const std::string& port, const std::string& doc_root);

			/// Run the server's io_service loop.
			void Run();

		private:
			/// Perform an asynchronous accept operation.
			void DoAccept();

			/// Wait for a request to stop the server.
			void DoAwaitStop();

			/// The io_service used to perform asynchronous operations.
			boost::asio::io_service m_IOService;

			/// The signal_set is used to register for process termination notifications.
			boost::asio::signal_set m_Signals;

			/// Acceptor used to listen for incoming connections.
			boost::asio::ip::tcp::acceptor m_Acceptor;
			
			/// The next socket to be accepted.
			boost::asio::ip::tcp::socket m_Socket;

			/// The connection manager which owns all live connections.
			ConnectionManager m_ConnectionManager;

			/// The handler for all incoming requests.
			RequestHandler m_RequestHandler;
		};

	} // namespace server
} // namespace http

#endif // HTTP_SERVER_HPP