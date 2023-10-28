#include "client.hpp"

#include <exception>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

auto main() -> int
{
	// ввод ip адреса сервера в локальной сети
	std::string ip	 {};
	std::string name {};

	std::cout << "ip: ";
	std::getline(std::cin, ip);
	std::cout << "name: ";
	std::getline(std::cin, name);

	try
	{
		nstu::Client::instance().connectToServer(name, ip, 2009, IPPROTO::IPPROTO_TCP);
	}
	catch (const std::exception& ex)
	{
		std::cout << "cannot connect to server due, " << ex.what();
		return 1;
	}

	return 0;
}
