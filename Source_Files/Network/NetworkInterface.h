/*
    Copyright (C) 2024 Benoit Hauquier and the "Aleph One" developers.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    This license is contained in the file "COPYING",
    which is included with this source code; it is available online at
    http://www.gnu.org/licenses/gpl.html
*/

#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

#include <asio.hpp>
#include <string>
#include <optional>
#include <tl/expected.hpp>

struct NetworkError {
    int code;
    std::string message;

    NetworkError(int code, std::string message) : code(code), message(message) {}
};

class IPaddress {
private:
    friend class NetworkInterface;
    friend class UDPsocket;
    friend class TCPsocket;

    asio::ip::address _address;
    uint16_t _port = 0;

    IPaddress(const asio::ip::tcp::endpoint& endpoint);
    IPaddress(const asio::ip::udp::endpoint& endpoint);

public:
    IPaddress(const std::string& host, uint16_t port);
    IPaddress(const uint8_t ip[4], uint16_t port);
    IPaddress() = default;
    bool is_v4() const { return _address.is_v4(); }
    std::string address() const { return _address.to_string(); }
    std::array<unsigned char, 4> address_bytes() const { return _address.to_v4().to_bytes(); }
    uint16_t port() const { return _port; }
    void set_port(uint16_t port);
    void set_address(const std::string& host);
    void set_address(const uint8_t ip[4]);

    bool operator==(const IPaddress& other) const;
    bool operator!=(const IPaddress& other) const;
};

/* missing from AppleTalk.h */
// ZZZ: note that this determines only the amount of storage allocated for packets, not
// the size of actual packets sent.  I believe UDP on Ethernet should be able to carry
// around 1.5K per packet, not sure of the exact figure off the top of my head though.
#define ddpMaxData 1500

struct UDPpacket
{
    IPaddress address;
    std::array<uint8_t, ddpMaxData> buffer;
    uint32_t data_size = 0;
};

class UDPsocket {
private:
    asio::io_context& _io_context;
    asio::ip::udp::socket _socket;
    asio::ip::udp::endpoint _receive_async_endpoint;
    asio::error_code _receive_async_error_code;
    uint64_t _receive_async_return_value = 0;
    UDPsocket(asio::io_context& io_context, asio::ip::udp::socket&& socket);
    friend class NetworkInterface;
public:
    tl::expected<uint64_t, NetworkError> broadcast_send(const UDPpacket& packet);
    tl::expected<uint64_t, NetworkError> send(const UDPpacket& packet);
    tl::expected<uint64_t, NetworkError> receive(UDPpacket& packet);
    void register_receive_async(UDPpacket& packet); //no error management here, it's done in the callback
    tl::expected<void, NetworkError> receive_async(int timeout_ms);
    tl::expected<void, NetworkError> broadcast(bool enable);
    tl::expected<uint64_t, NetworkError> check_receive() const;
};

class TCPsocket {
private:
    asio::io_context& _io_context;
    asio::ip::tcp::socket _socket;
    TCPsocket(asio::io_context& io_context, asio::ip::tcp::socket&& socket);
    friend class NetworkInterface;
    friend class TCPlistener;
public:
    tl::expected<uint64_t, NetworkError> send(uint8_t* buffer, size_t size);
    tl::expected<uint64_t, NetworkError> receive(uint8_t* buffer, size_t size);
    IPaddress remote_address() const { return IPaddress(_socket.remote_endpoint()); }
    tl::expected<void, NetworkError> set_non_blocking(bool enable);
};

class TCPlistener {
private:
    asio::io_context& _io_context;
    asio::ip::tcp::acceptor _acceptor;
    asio::ip::tcp::socket _socket;
    TCPlistener(asio::io_context& io_context, const asio::ip::tcp::endpoint& endpoint);
    friend class NetworkInterface;
public:
    tl::expected<std::unique_ptr<TCPsocket>, NetworkError> accept_connection();
    tl::expected<void, NetworkError> set_non_blocking(bool enable);
};

class NetworkInterface {
private:
    asio::io_context _io_context;
    asio::ip::tcp::resolver _resolver;
public:
    NetworkInterface();
    tl::expected<std::unique_ptr<UDPsocket>, NetworkError> udp_open_socket(uint16_t port);
    tl::expected<std::unique_ptr<TCPsocket>, NetworkError> tcp_connect_socket(const IPaddress& address);
    tl::expected<std::unique_ptr<TCPlistener>, NetworkError> tcp_open_listener(uint16_t port);
    tl::expected<std::optional<IPaddress>, NetworkError> resolve_address(const std::string& host, uint16_t port);
};

#endif // NETWORK_INTERFACE_H
