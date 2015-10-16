//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "stdafx.h"
#include "Connection.hpp"
#include <utility>
#include <vector>
#include "ConnectionManager.hpp"
#include "RequestHandler.hpp"

namespace Http
{
	namespace Server
	{

		Connection::Connection(boost::asio::ip::tcp::socket socket,
			ConnectionManager& manager, RequestHandler& handler)
			: m_Socket(std::move(socket)),
			m_ConnectionManager(manager),
			m_RequestHandler(handler)
		{
		}

		void Connection::Start()
		{
			DoRead();
		}

		void Connection::Stop()
		{
			m_Socket.close();
		}

		void Connection::DoRead()
		{
			auto self(shared_from_this());
			m_Socket.async_read_some(boost::asio::buffer(m_Buffer),
				[this, self](boost::system::error_code ec, std::size_t bytes_transferred)
			{
				if (!ec)
				{
					request_parser::result_type result;
					std::tie(result, std::ignore) = m_RequestParser.parse(
						m_Request, m_Buffer.data(), m_Buffer.data() + bytes_transferred);

					if (result == request_parser::good)
					{
						m_RequestHandler.HandleRequest(m_Request, m_Reply);
						DoWrite();
					}
					else if (result == request_parser::bad)
					{
						m_Reply = Reply::StockReply(Reply::ST_BAD_REQUEST);
						DoWrite();
					}
					else
					{
						DoRead();
					}
				}
				else if (ec != boost::asio::error::operation_aborted)
				{
					m_ConnectionManager.Stop(shared_from_this());
				}
			});
		}

		void Connection::DoWrite()
		{
			auto self(shared_from_this());
			boost::asio::async_write(m_Socket, m_Reply.ToBuffers(),
				[this, self](boost::system::error_code ec, std::size_t)
			{
				if (!ec)
				{
					// Initiate graceful connection closure.
					boost::system::error_code ignored_ec;
					m_Socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
						ignored_ec);
				}

				if (ec != boost::asio::error::operation_aborted)
				{
					m_ConnectionManager.Stop(shared_from_this());
				}
			});
		}

	} // namespace server
} // namespace http