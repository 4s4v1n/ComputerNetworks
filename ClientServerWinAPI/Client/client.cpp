#include "client.hpp"

#include <format>
#include <string>
#include <stdexcept>
#include <iostream>

Client::~Client()
{
	// закрытия существующего сокета
	if (m_server_socket != NULL)
	{
		closesocket(m_server_socket);
	}

	// остановка использования dll winsock
	WSACleanup();
}

auto Client::instance() -> Client&
{
	static Client local {};
	return local;
}

auto Client::connectToServer(const char* server_address, const std::uint16_t port, const IPPROTO& protocol) -> void
{
	WSADATA ws_data {};

	// инциализация использования dll winsock
	if (WSAStartup(MAKEWORD(2, 2), &ws_data) != 0)
	{
		throw std::runtime_error{std::format("wsa startup error {}", WSAGetLastError())};
	}

	// проверка индекса порта на валидность
	if (port < availible_port_index)
	{
		throw std::invalid_argument{std::format("port reserved by system, got {}, expected {}", 
			port, availible_port_index)};
	}

	// инциализация структуры для хранения адреса
	m_address.sin_addr.s_addr = inet_addr(server_address);
	m_address.sin_port		  = port;
	m_address.sin_family	  = AF_INET;
	
	// создание сокета для общения с сервером
	m_server_socket = socket(AF_INET, SOCK_STREAM, protocol);

	// установка соедениения с сокетом
	if (connect(m_server_socket, (LPSOCKADDR)&m_address, sizeof(m_address)) == SOCKET_ERROR)
	{
		throw std::runtime_error{std::format("cannot connect socekt, wsa error {}", WSAGetLastError())};
	}

	std::cout << "connected to server" << std::endl;

	while (true) {
		std::string message {};

		std::getline(std::cin, message);
		
		// отправка данных серверу (запрос)
		send(m_server_socket, message.c_str(), message.length(), NULL);
		
		if (message == "close.") {
			std::cout << "connection closed manually" << std::endl;
			break;
		}

		// получение данных от сервера (ответ)
		char buffer[1000] 	  {0};
		auto transferredBytes {recv(m_server_socket, buffer, sizeof(buffer), NULL)};

		if (transferredBytes == 0)
		{
			std::cout << "connection closed" << std::endl;
		}

		if (transferredBytes < 0)
		{
			throw std::runtime_error{std::format("recv runtime error, wsa error {}", WSAGetLastError())};
		}

		std::cout << buffer << std::endl;
	}
}
