//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include <string>

namespace Http
{
	namespace Server
	{
		struct Reply;
		struct Request;

		/// The common handler for all incoming requests.
		class RequestHandler
		{
		public:
			RequestHandler(const RequestHandler&) = delete;
			RequestHandler& operator=(const RequestHandler&) = delete;

			/// Construct with a directory containing files to be served.
			explicit RequestHandler(const std::string& doc_root);

			/// Handle a request and produce a reply.
			void HandleRequest(const Request& req, Reply& rep);

		private:
			/// The directory containing the files to be served.
			std::string m_DocumentRoot;

			/// Perform URL-decoding on a string. Returns false if the encoding was
			/// invalid.
			static bool DecodeURL(const std::string& in, std::string& out);
		};

	} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HANDLER_HPP