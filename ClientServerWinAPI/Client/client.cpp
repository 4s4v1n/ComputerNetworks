#include "client.hpp"

#include <format>
#include <string>
#include <stdexcept>
#include <iostream>

Client::~Client()
{
	if (m_socket != NULL)
	{
		closesocket(m_socket);
	}

	WSACleanup();
}

auto Client::instance() -> Client&
{
	static Client local {};
	return local;
}

auto Client::connectToServer(const char* server_address, const std::uint16_t port) -> void
{
	WSADATA ws_data {};

	if (WSAStartup(MAKEWORD(2, 2), &ws_data) != 0)
	{
		throw std::runtime_error{std::format("wsa startup error {}", WSAGetLastError())};
	}

	if (port < availible_port_index)
	{
		throw std::invalid_argument{std::format("port reserved by system, got {}, expected {}", 
			port, availible_port_index)};
	}

	m_address.sin_addr.s_addr = inet_addr(server_address);
	m_address.sin_port		  = port;
	m_address.sin_family	  = AF_INET;

	m_socket = socket(AF_INET, SOCK_STREAM, NULL);

	if (connect(m_socket, (LPSOCKADDR)&m_address, sizeof(m_address)) == SOCKET_ERROR)
	{
		throw std::runtime_error{ std::format("cannot connect socekt, wsa error {}", WSAGetLastError())};
	}

	std::cout << "connected to server" << std::endl;

	while (true) {
		std::string message {};

		std::getline(std::cin, message);
		send(m_socket, message.c_str(), message.length(), NULL);
	}
}