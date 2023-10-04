#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdint>

#include <WinSock2.h>

// синглтон
class Client
{
private:
	Client() = default;
	~Client();
	Client(const Client&) = delete;
	Client(Client&&) = delete;

	auto operator=(const Client&) -> Client& = delete;
	auto operator=(Client&&) -> Client& = delete;

private:
	static constexpr std::uint16_t availible_port_index {1024};

public:
	static auto instance() -> Client&;
	auto connectToServer(const char* server_address, const std::uint16_t port, const IPPROTO& protocol) -> void;

private:
	SOCKADDR_IN m_address 		{};
	SOCKET      m_server_socket {NULL};
};

#endif // SERVER_HPP
