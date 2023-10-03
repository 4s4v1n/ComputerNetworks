#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstdint>
#include <string>

#include <WinSock2.h>

class Server
{
public:
	enum class Protocol
	{
		TCP,
		UDP
	};

private:
	Server() = default;
	~Server();
	Server(const Server&) = delete;
	Server(Server&&) = delete;

	auto operator=(const Server&) -> Server& = delete;
	auto operator=(Server&&) -> Server& = delete;

private:
	static constexpr std::uint16_t availible_port_index		 { 1024 };
	static constexpr std::uint8_t  max_availible_connections { 3 };
	static constexpr const char*   localhost				 { "127.0.0.1" };
	static constexpr const char*   sentenseFinish            { ".!?" };
	static constexpr const char*   sentenseDelimiter         { "\t\n" };

private:
	auto addToTextLettersCount(const std::string& text) const noexcept -> std::string;

public:
	static auto instance() -> Server&;
	auto init(const std::uint16_t port, const Protocol& protocol) -> void;
	auto run() -> void;

private:
	SOCKADDR_IN	  m_address		  {};
	SOCKET		  m_listen_socket {NULL};
	SOCKET		  m_client_socket {NULL};
};

#endif // SERVER_HPP
