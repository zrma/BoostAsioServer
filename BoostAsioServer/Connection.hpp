//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include <array>
#include <memory>
#include <boost/asio.hpp>
#include "Reply.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"
#include "RequestParser.hpp"

namespace Http
{
	namespace Server 
	{
		class ConnectionManager;

		/// Represents a single connection from a client.
		class Connection
			: public std::enable_shared_from_this<Connection>
		{
		public:
			Connection(const Connection&) = delete;
			Connection& operator=(const Connection&) = delete;

			/// Construct a connection with the given socket.
			explicit Connection(boost::asio::ip::tcp::socket socket,
				ConnectionManager& manager, RequestHandler& handler);

			/// Start the first asynchronous operation for the connection.
			void Start();

			/// Stop all asynchronous operations associated with the connection.
			void Stop();

		private:
			/// Perform an asynchronous read operation.
			void DoRead();

			/// Perform an asynchronous write operation.
			void DoWrite();

			/// Socket for the connection.
			boost::asio::ip::tcp::socket m_Socket;

			/// The manager for this connection.
			ConnectionManager& m_ConnectionManager;

			/// The handler used to process the incoming request.
			RequestHandler& m_RequestHandler;

			/// Buffer for incoming data.
			std::array<char, 8192> m_Buffer;

			/// The incoming request.
			Request m_Request;

			/// The parser for the incoming request.
			request_parser m_RequestParser;

			/// The reply to be sent back to the client.
			Reply m_Reply;
		};

		typedef std::shared_ptr<Connection> ConnectionPtr;

	} // namespace server
} // namespace http

#endif // HTTP_CONNECTION_HPP