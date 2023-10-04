#include "client.hpp"

#include <exception>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

auto main() -> int
{
	try
	{
		Client::instance().connectToServer("127.0.0.1", 2009, IPPROTO::IPPROTO_TCP);
	}
	catch (const std::exception& ex)
	{
		std::cout << "cannot connect to server due, " << ex.what();
		return 1;
	}

	return 0;
}