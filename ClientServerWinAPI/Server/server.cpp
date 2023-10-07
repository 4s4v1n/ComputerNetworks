#include "server.hpp"

#include <format>
#include <stdexcept>
#include <iostream>

Server::~Server()
{
	// закрытия существующих сокетов
	if (m_client_socket != NULL)
	{
		closesocket(m_client_socket);
	}

	if (m_listen_socket != NULL)
	{
		closesocket(m_listen_socket);
	}

	// остановка использования dll winsock
	WSACleanup();
}

auto Server::instance() -> Server&
{
	static Server local {};
	return local;
}

auto Server::init(const std::uint16_t port, const IPPROTO& protocol) -> void
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
		throw std::invalid_argument{std::format("port reserved by system, got {}, expected >= {}", 
			port, availible_port_index)};
	}

	int address_length {sizeof(m_address)};

	// инциализация структуры для хранения адреса
	m_address.sin_addr.s_addr = INADDR_ANY;
	m_address.sin_port		  = htons(port);
	m_address.sin_family	  = AF_INET;

	// создание сокета для прослушивания входящих запросов на соединение
	m_listen_socket = socket(AF_INET, SOCK_STREAM, protocol);
	if (m_listen_socket == INVALID_SOCKET)
	{
		throw std::runtime_error{std::format("cannot create socket, wsa error {}", WSAGetLastError())};
	}
}

auto Server::run() -> void
{
	int address_length {sizeof(m_address)};

	// привязка сокета к адресу
	if (bind(m_listen_socket, (SOCKADDR*)&m_address, address_length) == SOCKET_ERROR)
	{
		throw std::runtime_error{std::format("cannot bind socket, wsa error {}", WSAGetLastError())};
	}
	// прослушивание {ip}:{port} сокетом, на входящие соединения (1)
	if (listen(m_listen_socket, max_availible_connections))
	{
		throw std::runtime_error{std::format("cannot listen socket, wsa error {}", WSAGetLastError())};
	}

	char host[256]       {0};
	char host_name[1024] {0};

	// получение адреса сервера в локальной сети
	if (!gethostname(host_name, 1024))
	{
		if (LPHOSTENT lp_host {gethostbyname(host_name)})
		{
			std::strcpy(host, inet_ntoa(*((in_addr*)lp_host->h_addr_list[0])));
		}
	}
	std::cout << std::format("server started at {}:{}", host, htons(m_address.sin_port)) << std::endl;

	// извлечение первой входящей попытки на подлкючение к сокету
	m_client_socket = accept(m_listen_socket, (SOCKADDR*)&m_address, &address_length);

	if (m_client_socket == NULL)
	{
		throw std::runtime_error{std::format("cannot accept connection, wsa error {}", WSAGetLastError())};
	}

	std::cout << "client connected" << std::endl;

	while (true) {
		// получение данных (запрос) от клиента
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
		if (text == "close.")
		{
			std::cout << "connection closed manually" << std::endl;
			break;
		}

		// обработка текста в соответсвии с вариантом
		std::string processed{addToTextLettersCount(text)};

		// отправка клиенту форматированного текста (ответа)
		send(m_client_socket, processed.c_str(), processed.length(), NULL);
	}
}

auto Server::addToTextLettersCount(const std::string& text) const noexcept -> std::string
{
	auto		start     {0};
	auto		finish    {text.find_first_of(sentense_finish, start)};
	std::string proccesed {};

	while (finish != std::string::npos)
	{
		proccesed += text.substr(start, finish - start) + " " + std::to_string(finish - start) + text[finish];
		start 	  =  text.find_first_not_of(sentense_delimiter, finish + 1);
		finish 	  =  text.find_first_of(sentense_finish, start);
	}

	return proccesed;
}
