#include "server.hpp"

#include <exception>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

auto main() -> int
{
	try
	{
		Server::instance().init(2009, IPPROTO::IPPROTO_TCP);
		Server::instance().run();
	}
	catch (const std::exception& ex)
	{
		std::cout << "server error: " << ex.what();
		return 1;
	}

	return 0;
}