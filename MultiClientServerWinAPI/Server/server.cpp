#include "server.hpp"

#include <csignal>
#include <format>
#include <iostream>
#include <stdexcept>
#include <thread>

#include <Windows.h>

namespace nstu
{

Server::~Server()
{
	// ожидание завершения работы потоков
	for (auto& thread : m_handler_threads)
	{
		thread.join();
	}

	// закрытия существующих сокетов
	for (auto& [id, socket] : m_client_sockets)
	{
		if (socket != NULL)
		{
			closesocket(socket);
		}
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
		throw std::runtime_error {std::format("wsa startup error {}",
			WSAGetLastError())};
	}

	// проверка индекса порта на валидность
	if (port < availible_port_index)
	{
		throw std::invalid_argument {std::format("port reserved by system, got {}, expected >= {}", 
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
		throw std::runtime_error {std::format("cannot create socket, wsa error {}",
			WSAGetLastError())};
	}
}

auto Server::run() -> void
{
	int address_length {sizeof(m_address)};

	// привязка сокета к адресу
	if (bind(m_listen_socket, (SOCKADDR*)&m_address, address_length) == SOCKET_ERROR)
	{
		throw std::runtime_error {std::format("cannot bind socket, wsa error {}", 
			WSAGetLastError())};
	}
	// прослушивание {ip}:{port} сокетом, на входящие соединения (1)
	if (listen(m_listen_socket, SOMAXCONN))
	{
		throw std::runtime_error {std::format("cannot listen socket, wsa error {}",
			WSAGetLastError())};
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

	int id {1};
	while(true)
	{
		// извлечение первой входящей попытки на подлкючение к сокету
		SOCKET socket {accept(m_listen_socket, (SOCKADDR*)&m_address, &address_length)};

		if (socket == NULL)
		{
			throw std::runtime_error {std::format("cannot accept connection, wsa error {}",
				WSAGetLastError())};
		}

		std::cout << std::format("client {} connected", id)  << std::endl;

		m_client_sockets[id] = socket;
		m_handler_threads.emplace_back(std::move(
			std::thread(&Server::clientHandler, this, std::ref(m_client_sockets), id++)));
	}
}

// обработчик текста
auto Server::addToTextLettersCount(const std::string& text) const noexcept -> std::string
{
	auto		start     {0};
	auto		finish    {text.find_first_of(Server::sentense_finish, start)};
	std::string proccesed {};

	while (finish != std::string::npos)
	{
		proccesed += text.substr(start, finish - start) + " " + 
			std::to_string(finish - start) + text[finish];
		start     =  text.find_first_not_of(Server::sentense_delimiter, finish + 1);
		finish    =  text.find_first_of(Server::sentense_finish, start);
	}

	return proccesed;
}

// обработчик запросов от 1го клиента
auto Server::clientHandler(std::unordered_map<int, SOCKET>& sockets, const int id) -> void
{
	while (true)
	{
		// получение данных (запрос) от клиента
		char buffer[1000]     {0};
		auto transferredBytes {recv(sockets[id], buffer, sizeof(buffer), NULL)};

		if (transferredBytes == 0)
		{
			std::cout << "connection closed" << std::endl;
			break;
		}

		if (transferredBytes < 0)
		{
			std::cout << std::format("connection from client {} closed", id) << std::endl;
			break;
		}

		std::string text {buffer};
		if (text == stop_word)
		{
			std::cout << std::format("connection from client {} closed", id) << std::endl;
			break;
		} 

		// обработка текста в соответсвии с вариантом
		std::string processed {addToTextLettersCount(text)};

		std::cout << std::format("processed request from client {}", id) << std::endl;

		// блокировка мьютекса до конца области видимости 
		const std::lock_guard<std::mutex> lock{ m_mutex };
		for (const auto& [id, socket] : sockets)
		{
			// отправка клиенту форматированного текста (ответа)
			if (send(socket,
				!processed.empty() ? processed.c_str() : "",
				!processed.empty() ? processed.length() : 1, NULL) != SOCKET_ERROR)
			{
				std::cout << std::format("send request to client {}", id) << std::endl;
			}
		}
	}

	// блокировка мьютекса до конца области видимости ф-ции
	const std::lock_guard<std::mutex> lock{ m_mutex };

	// закрытие и удаление сокета соответсвующего клиенту
	closesocket(m_client_sockets[id]); 
	m_client_sockets.erase(id);
}

} // namespace nstu
