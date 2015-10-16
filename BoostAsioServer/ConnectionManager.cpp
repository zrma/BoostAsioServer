//
// connection_manager.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "stdafx.h"
#include "ConnectionManager.hpp"

namespace Http
{
	namespace Server
	{
		ConnectionManager::ConnectionManager()
		{
		}

		void ConnectionManager::Start(ConnectionPtr c)
		{
			m_Connections.insert(c);
			c->Start();
		}

		void ConnectionManager::Stop(ConnectionPtr c)
		{
			m_Connections.erase(c);
			c->Stop();
		}

		void ConnectionManager::StopAll()
		{
			for (auto c : m_Connections)
				c->Stop();
			m_Connections.clear();
		}

	} // namespace server
} // namespace http