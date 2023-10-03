#include "server.hpp"

#include <format>
#include <stdexcept>
#include <iostream>

Server::~Server()
{
	if (m_client_socket != NULL)
	{
		closesocket(m_client_socket);
	}

	if (m_listen_socket != NULL)
	{
		closesocket(m_listen_socket);
	}

	WSACleanup();
}

auto Server::instance() -> Server&
{
	static Server local {};
	return local;
}

auto Server::init(const std::uint16_t port, const Protocol& protocol) -> void
{
	WSADATA ws_data {};

	if (WSAStartup(MAKEWORD(2, 2), &ws_data) != 0)
	{
		throw std::runtime_error{std::format("wsa startup error {}", WSAGetLastError())};
	}


	if (port < availible_port_index)
	{
		throw std::invalid_argument{std::format("port reserved by system, got {}, expected >= {}", 
			port, availible_port_index)};
	}

	int address_length { sizeof(m_address) };

	m_address.sin_addr.s_addr = inet_addr(localhost);
	m_address.sin_port		  = port;
	m_address.sin_family	  = AF_INET;

	m_listen_socket = socket(AF_INET, SOCK_STREAM, NULL);
	if (m_listen_socket == INVALID_SOCKET)
	{
		throw std::runtime_error{ std::format("cannot create socket, wsa error {}", WSAGetLastError())};
	}
}

auto Server::run() -> void
{
	int address_length { sizeof(m_address) };

	if (bind(m_listen_socket, (SOCKADDR*)&m_address, address_length) == SOCKET_ERROR)
	{
		throw std::runtime_error{std::format("cannot bind socket, wsa error {}", WSAGetLastError())};
	}
	if (listen(m_listen_socket, SOMAXCONN))
	{
		throw std::runtime_error{std::format("cannot listen socket, wsa error {}", WSAGetLastError())};
	}

	std::cout << "server started" << std::endl;

	m_client_socket = accept(m_listen_socket, (SOCKADDR*)&m_address, &address_length);

	if (m_client_socket == NULL)
	{
		throw std::runtime_error{std::format("cannot accept connection, wsa error {}", WSAGetLastError())};
	}

	std::cout << "client connected" << std::endl;

	while (true) {
		char buffer[1000]     {0};
		auto transferredBytes {recv(m_client_socket, buffer, sizeof(buffer), NULL)};

		if (transferredBytes == 0)
		{
			std::cout << "connection closed" << std::endl;
			break;
		}

		if (transferredBytes < 0)
		{
			throw std::runtime_error{std::format("recv runtime error, wsa error {}", WSAGetLastError())};
		}

		std::string text {buffer};
		if (text == "close")
		{
			std::cout << "connection closed manually";
			break;
		}

		std::cout << addToTextLettersCount(trxt) << std::endl;
	}
}

auto Server::addToTextLettersCount(const std::string& text) const noexcept -> std::string
{
	auto		start     {0};
	auto		finish    {text.find_first_of(sentenseFinish, start)};
	std::string proccesed {};

	while (finish != std::string::npos)
	{
		proccesed += text.substr(start, finish - start) + " " + std::to_string(finish - start) + text[finish];
		start = text.find_first_not_of(sentenseDelimiter, finish + 1);
		finish = text.find_first_of(sentenseFinish, start);
	}

	return proccesed;
}