//
// mime_types.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "stdafx.h"
#include "MimeTypes.hpp"

namespace Http
{
	namespace Server
	{
		namespace MimeTypes
		{
			struct Mapping
			{
				const char* m_Extension;
				const char* m_MimeType;
			} m_Mappings[] =
			{
				{ "gif", "image/gif" },
				{ "htm", "text/html" },
				{ "html", "text/html" },
				{ "jpg", "image/jpeg" },
				{ "png", "image/png" }
			};

			std::string ExtensionToType(const std::string& extension)
			{
				for (Mapping m : m_Mappings)
				{
					if (m.m_Extension == extension)
					{
						return m.m_MimeType;
					}
				}

				return "text/plain";
			}

		} // namespace mime_types
	} // namespace server
} // namespace http