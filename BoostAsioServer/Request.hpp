//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <vector>
#include "Header.hpp"

namespace Http
{
	namespace Server
	{
		/// A request received from a client.
		struct Request
		{
			std::string m_Method;
			std::string m_URI;
			int m_HttpVersionMajor;
			int m_HttpVersionMinor;
			std::vector<Header> m_Headers;
		};

	} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HPP