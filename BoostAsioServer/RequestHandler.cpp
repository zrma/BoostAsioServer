//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "stdafx.h"
#include "RequestHandler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include "MimeTypes.hpp"
#include "Reply.hpp"
#include "Request.hpp"

namespace Http
{
	namespace Server
	{
		RequestHandler::RequestHandler(const std::string& doc_root)
			: m_DocumentRoot(doc_root)
		{
		}

		void RequestHandler::HandleRequest(const Request& req, Reply& rep)
		{
			// Decode url to path.
			std::string request_path;
			if (!DecodeURL(req.m_URI, request_path))
			{
				rep = Reply::StockReply(Reply::ST_BAD_REQUEST);
				return;
			}

			// Request path must be absolute and not contain "..".
			if (request_path.empty() || request_path[0] != '/'
				|| request_path.find("..") != std::string::npos)
			{
				rep = Reply::StockReply(Reply::ST_BAD_REQUEST);
				return;
			}

			// If path ends in slash (i.e. is a directory) then add "index.html".
			if (request_path[request_path.size() - 1] == '/')
			{
				request_path += "index.html";
			}

			// Determine the file extension.
			std::size_t last_slash_pos = request_path.find_last_of("/");
			std::size_t last_dot_pos = request_path.find_last_of(".");
			std::string extension;
			if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
			{
				extension = request_path.substr(last_dot_pos + 1);
			}

			// Open the file to send back.
			std::string full_path = m_DocumentRoot + request_path;
			std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
			if (!is)
			{
				rep = Reply::StockReply(Reply::ST_NOT_FOUND);
				return;
			}

			// Fill out the reply to be sent to the client.
			rep.m_Status = Reply::ST_OK;
			char buf[512];
			while (is.read(buf, sizeof(buf)).gcount() > 0)
				rep.m_Content.append(buf, is.gcount());
			rep.m_Headers.resize(2);
			rep.m_Headers[0].m_Name = "Content-Length";
			rep.m_Headers[0].m_Value = std::to_string(rep.m_Content.size());
			rep.m_Headers[1].m_Name = "Content-Type";
			rep.m_Headers[1].m_Value = MimeTypes::ExtensionToType(extension);
		}

		bool RequestHandler::DecodeURL(const std::string& in, std::string& out)
		{
			out.clear();
			out.reserve(in.size());
			for (std::size_t i = 0; i < in.size(); ++i)
			{
				if (in[i] == '%')
				{
					if (i + 3 <= in.size())
					{
						int value = 0;
						std::istringstream is(in.substr(i + 1, 2));
						if (is >> std::hex >> value)
						{
							out += static_cast<char>(value);
							i += 2;
						}
						else
						{
							return false;
						}
					}
					else
					{
						return false;
					}
				}
				else if (in[i] == '+')
				{
					out += ' ';
				}
				else
				{
					out += in[i];
				}
			}
			return true;
		}

	} // namespace server
} // namespace http