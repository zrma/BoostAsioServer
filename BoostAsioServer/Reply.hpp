//
// reply.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_REPLY_HPP
#define HTTP_REPLY_HPP

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "Header.hpp"

namespace Http
{
	namespace Server
	{
		/// A reply to be sent to a client.
		struct Reply
		{
			/// The status of the reply.
			enum StatusType
			{
				ST_OK = 200,
				ST_CREATED = 201,
				ST_ACCEPTED = 202,
				ST_NO_CONTENT = 204,
				ST_MULTIPLE_CHOICES = 300,
				ST_MOVED_PERMANENTLY = 301,
				ST_MOVED_TEMPORARILY = 302,
				ST_NOT_MODIFIED = 304,
				ST_BAD_REQUEST = 400,
				ST_UNAUTHORIZED = 401,
				ST_FORBIDDEN = 403,
				ST_NOT_FOUND = 404,
				ST_INTERNAL_SERVER_ERROR = 500,
				ST_NOT_IMPLEMENTED = 501,
				ST_BAD_GATEWAY = 502,
				ST_SERVICE_UNAVAILABLE = 503
			} m_Status;

			/// The headers to be included in the reply.
			std::vector<Header> m_Headers;

			/// The content to be sent in the reply.
			std::string m_Content;

			/// Convert the reply into a vector of buffers. The buffers do not own the
			/// underlying memory blocks, therefore the reply object must remain valid and
			/// not be changed until the write operation has completed.
			std::vector<boost::asio::const_buffer> ToBuffers();

			/// Get a stock reply.
			static Reply StockReply(StatusType status);
		};

	} // namespace server
} // namespace http

#endif // HTTP_REPLY_HPP