#include "client.hpp"

#include <exception>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

auto main() -> int
{
	// ввод ip адреса сервера в локальной сети
	std::string ip{};
	std::cout << "ip: ";
	std::cin >> ip;

	std::cin.ignore();

	try
	{
		nstu::Client::instance().connectToServer(ip.c_str(), 2009, IPPROTO::IPPROTO_TCP);
	}
	catch (const std::exception& ex)
	{
		std::cout << "cannot connect to server due, " << ex.what();
		return 1;
	}

	return 0;
}