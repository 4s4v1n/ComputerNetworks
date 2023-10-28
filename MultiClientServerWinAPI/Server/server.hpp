#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>

#include <WinSock2.h>

namespace nstu
{
// синглтон
class Server
{
private:
	struct ClientData
	{
		SOCKET      socket	{};
		std::string address {};
		std::string name	{};
	};

private:
	Server() = default;
	~Server();
	Server(const Server&) = delete;
	Server(Server&&) = delete;

	auto operator=(const Server&) -> Server& = delete;
	auto operator=(Server&&) -> Server& = delete;

private:
	static constexpr std::uint16_t availible_port_index {1024};
	static constexpr const char*   stop_word			{"stop"};

private:
	auto clientHandler(std::unordered_map<int, ClientData>& clients, const int id) -> void;

public:
	static auto instance() -> Server&;
	auto init(const std::uint16_t port, const IPPROTO& protocol) -> void;
	auto run() -> void;

private:
	SOCKADDR_IN m_address		{};
	SOCKET		m_listen_socket {NULL};
	std::mutex	m_mutex		    {};

	std::vector<std::thread>			m_handler_threads {};
	std::unordered_map<int, ClientData> m_clients	      {};
};

} // namespace nstu

#endif // SERVER_HPP
