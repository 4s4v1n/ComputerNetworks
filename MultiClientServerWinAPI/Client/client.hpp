#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdint>
#include <mutex>
#include <thread>

#include <WinSock2.h>

namespace nstu
{
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
	static constexpr const char*   stop_word		    {"stop"};

public:
	static auto instance() -> Client&;
	auto connectToServer(const std::string& name, 
		const std::string& address, const std::uint16_t port,
		const IPPROTO& protocol) -> void;

private:
	SOCKADDR_IN m_address 		{};
	SOCKET      m_server_socket {NULL};
	
	std::mutex  m_mutex			  {};
	std::thread m_reciving_thread {};
};

} // namespace nstu

#endif // CLIENT_HPP
