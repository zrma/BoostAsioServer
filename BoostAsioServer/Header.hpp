//
// header.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_HEADER_HPP
#define HTTP_HEADER_HPP

#include <string>

namespace Http
{
	namespace Server
	{
		struct Header
		{
			std::string m_Name;
			std::string m_Value;
		};

	} // namespace server
} // namespace http

#endif HTTP_HEADER_HPP