#include "server.hpp"

#include <exception>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

auto main() -> int
{
	try
	{
		nstu::Server::instance().init(2009, IPPROTO::IPPROTO_TCP);
		nstu::Server::instance().run();
	}
	catch (const std::exception& ex)
	{
		std::cerr << "server error: " << ex.what();
		return 1;
	}

	return 0;
}
