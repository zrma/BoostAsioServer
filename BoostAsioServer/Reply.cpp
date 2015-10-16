//
// reply.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "stdafx.h"
#include "Reply.hpp"
#include <string>

namespace Http
{
	namespace Server
	{
		namespace StatusStrings
		{
			const std::string ok =
				"HTTP/1.0 200 OK\r\n";
			const std::string created =
				"HTTP/1.0 201 Created\r\n";
			const std::string accepted =
				"HTTP/1.0 202 Accepted\r\n";
			const std::string no_content =
				"HTTP/1.0 204 No Content\r\n";
			const std::string multiple_choices =
				"HTTP/1.0 300 Multiple Choices\r\n";
			const std::string moved_permanently =
				"HTTP/1.0 301 Moved Permanently\r\n";
			const std::string moved_temporarily =
				"HTTP/1.0 302 Moved Temporarily\r\n";
			const std::string not_modified =
				"HTTP/1.0 304 Not Modified\r\n";
			const std::string bad_request =
				"HTTP/1.0 400 Bad Request\r\n";
			const std::string unauthorized =
				"HTTP/1.0 401 Unauthorized\r\n";
			const std::string forbidden =
				"HTTP/1.0 403 Forbidden\r\n";
			const std::string not_found =
				"HTTP/1.0 404 Not Found\r\n";
			const std::string internal_server_error =
				"HTTP/1.0 500 Internal Server Error\r\n";
			const std::string not_implemented =
				"HTTP/1.0 501 Not Implemented\r\n";
			const std::string bad_gateway =
				"HTTP/1.0 502 Bad Gateway\r\n";
			const std::string service_unavailable =
				"HTTP/1.0 503 Service Unavailable\r\n";

			boost::asio::const_buffer ToBuffer(Reply::StatusType status)
			{
				switch (status)
				{
				case Reply::ST_OK:
					return boost::asio::buffer(ok);
				case Reply::ST_CREATED:
					return boost::asio::buffer(created);
				case Reply::ST_ACCEPTED:
					return boost::asio::buffer(accepted);
				case Reply::ST_NO_CONTENT:
					return boost::asio::buffer(no_content);
				case Reply::ST_MULTIPLE_CHOICES:
					return boost::asio::buffer(multiple_choices);
				case Reply::ST_MOVED_PERMANENTLY:
					return boost::asio::buffer(moved_permanently);
				case Reply::ST_MOVED_TEMPORARILY:
					return boost::asio::buffer(moved_temporarily);
				case Reply::ST_NOT_MODIFIED:
					return boost::asio::buffer(not_modified);
				case Reply::ST_BAD_REQUEST:
					return boost::asio::buffer(bad_request);
				case Reply::ST_UNAUTHORIZED:
					return boost::asio::buffer(unauthorized);
				case Reply::ST_FORBIDDEN:
					return boost::asio::buffer(forbidden);
				case Reply::ST_NOT_FOUND:
					return boost::asio::buffer(not_found);
				case Reply::ST_INTERNAL_SERVER_ERROR:
					return boost::asio::buffer(internal_server_error);
				case Reply::ST_NOT_IMPLEMENTED:
					return boost::asio::buffer(not_implemented);
				case Reply::ST_BAD_GATEWAY:
					return boost::asio::buffer(bad_gateway);
				case Reply::ST_SERVICE_UNAVAILABLE:
					return boost::asio::buffer(service_unavailable);
				default:
					return boost::asio::buffer(internal_server_error);
				}
			}

		} // namespace status_strings

		namespace MiscStrings {

			const char name_value_separator[] = { ':', ' ' };
			const char crlf[] = { '\r', '\n' };

		} // namespace misc_strings

		std::vector<boost::asio::const_buffer> Reply::ToBuffers()
		{
			std::vector<boost::asio::const_buffer> buffers;
			buffers.push_back(StatusStrings::ToBuffer(m_Status));
			for (std::size_t i = 0; i < m_Headers.size(); ++i)
			{
				Header& h = m_Headers[i];
				buffers.push_back(boost::asio::buffer(h.m_Name));
				buffers.push_back(boost::asio::buffer(MiscStrings::name_value_separator));
				buffers.push_back(boost::asio::buffer(h.m_Value));
				buffers.push_back(boost::asio::buffer(MiscStrings::crlf));
			}
			buffers.push_back(boost::asio::buffer(MiscStrings::crlf));
			buffers.push_back(boost::asio::buffer(m_Content));
			return buffers;
		}

		namespace StockReplies {

			const char ok[] = "";
			const char created[] =
				"<html>"
				"<head><title>Created</title></head>"
				"<body><h1>201 Created</h1></body>"
				"</html>";
			const char accepted[] =
				"<html>"
				"<head><title>Accepted</title></head>"
				"<body><h1>202 Accepted</h1></body>"
				"</html>";
			const char no_content[] =
				"<html>"
				"<head><title>No Content</title></head>"
				"<body><h1>204 Content</h1></body>"
				"</html>";
			const char multiple_choices[] =
				"<html>"
				"<head><title>Multiple Choices</title></head>"
				"<body><h1>300 Multiple Choices</h1></body>"
				"</html>";
			const char moved_permanently[] =
				"<html>"
				"<head><title>Moved Permanently</title></head>"
				"<body><h1>301 Moved Permanently</h1></body>"
				"</html>";
			const char moved_temporarily[] =
				"<html>"
				"<head><title>Moved Temporarily</title></head>"
				"<body><h1>302 Moved Temporarily</h1></body>"
				"</html>";
			const char not_modified[] =
				"<html>"
				"<head><title>Not Modified</title></head>"
				"<body><h1>304 Not Modified</h1></body>"
				"</html>";
			const char bad_request[] =
				"<html>"
				"<head><title>Bad Request</title></head>"
				"<body><h1>400 Bad Request</h1></body>"
				"</html>";
			const char unauthorized[] =
				"<html>"
				"<head><title>Unauthorized</title></head>"
				"<body><h1>401 Unauthorized</h1></body>"
				"</html>";
			const char forbidden[] =
				"<html>"
				"<head><title>Forbidden</title></head>"
				"<body><h1>403 Forbidden</h1></body>"
				"</html>";
			const char not_found[] =
				"<html>"
				"<head><title>Not Found</title></head>"
				"<body><h1>404 Not Found</h1></body>"
				"</html>";
			const char internal_server_error[] =
				"<html>"
				"<head><title>Internal Server Error</title></head>"
				"<body><h1>500 Internal Server Error</h1></body>"
				"</html>";
			const char not_implemented[] =
				"<html>"
				"<head><title>Not Implemented</title></head>"
				"<body><h1>501 Not Implemented</h1></body>"
				"</html>";
			const char bad_gateway[] =
				"<html>"
				"<head><title>Bad Gateway</title></head>"
				"<body><h1>502 Bad Gateway</h1></body>"
				"</html>";
			const char service_unavailable[] =
				"<html>"
				"<head><title>Service Unavailable</title></head>"
				"<body><h1>503 Service Unavailable</h1></body>"
				"</html>";

			std::string ToString(Reply::StatusType status)
			{
				switch (status)
				{
				case Reply::ST_OK:
					return ok;
				case Reply::ST_CREATED:
					return created;
				case Reply::ST_ACCEPTED:
					return accepted;
				case Reply::ST_NO_CONTENT:
					return no_content;
				case Reply::ST_MULTIPLE_CHOICES:
					return multiple_choices;
				case Reply::ST_MOVED_PERMANENTLY:
					return moved_permanently;
				case Reply::ST_MOVED_TEMPORARILY:
					return moved_temporarily;
				case Reply::ST_NOT_MODIFIED:
					return not_modified;
				case Reply::ST_BAD_REQUEST:
					return bad_request;
				case Reply::ST_UNAUTHORIZED:
					return unauthorized;
				case Reply::ST_FORBIDDEN:
					return forbidden;
				case Reply::ST_NOT_FOUND:
					return not_found;
				case Reply::ST_INTERNAL_SERVER_ERROR:
					return internal_server_error;
				case Reply::ST_NOT_IMPLEMENTED:
					return not_implemented;
				case Reply::ST_BAD_GATEWAY:
					return bad_gateway;
				case Reply::ST_SERVICE_UNAVAILABLE:
					return service_unavailable;
				default:
					return internal_server_error;
				}
			}

		} // namespace stock_replies

		Reply Reply::StockReply(Reply::StatusType status)
		{
			Reply rep;
			rep.m_Status = status;
			rep.m_Content = StockReplies::ToString(status);
			rep.m_Headers.resize(2);
			rep.m_Headers[0].m_Name = "Content-Length";
			rep.m_Headers[0].m_Value = std::to_string(rep.m_Content.size());
			rep.m_Headers[1].m_Name = "Content-Type";
			rep.m_Headers[1].m_Value = "text/html";
			return rep;
		}

	} // namespace server
} // namespace http