#ifndef SendReceive_H
#define SendReceive_H

#include <boost/asio.hpp>
#include <functional>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace asio = boost::asio;

class MulticasterSender {
   public:
    MulticasterSender(asio::io_context& ioContext,
                      const std::string multicastAddress,
                      unsigned short multicastPort);

    void send(const json& jsonData);

    unsigned short getPort();

    std::string getAddress();

   private:
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint multicastEndpoint_;
    std::string multicastAddress_;
    unsigned short multicastPort_;
};

class MulticasterReceiver {
   public:
    using ProcessJson = std::function<void(const json&)>;

    MulticasterReceiver(asio::io_context& ioContext,
                        const std::string multicastAddress,
                        unsigned short multicastPort, ProcessJson f);
    void receive();

    std::string getAddress();

    unsigned short getPort();

   private:
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint multicastEndpoint_;
    std::string multicastAddress_;
    unsigned short multicastPort_;
    ProcessJson processJson_;
};

class TCPClient {
   public:
    TCPClient(asio::io_context& ioContext, const std::string serverAddress,
              const std::string port);

    void send(const json& jsonData);

   private:
    asio::ip::tcp::socket socket_;
    asio::streambuf buffer_;
    std::string serverAddress_;
    std::string port_;
};

class TCPSession : public std::enable_shared_from_this<TCPSession> {
   public:
    using ProcessJson = std::function<void(const json&)>;
    TCPSession(asio::ip::tcp::socket socket, ProcessJson f);

    void start();

   private:
    asio::ip::tcp::socket socket_;
    asio::streambuf buffer_;
    ProcessJson processJson_;
};

class TCPServer {
   public:
    using ProcessJson = std::function<void(const json&)>;

    TCPServer(asio::io_context& ioContext, asio::ip::tcp address,
              unsigned short port, ProcessJson f);

    TCPServer(asio::io_context& ioContext, unsigned short port, ProcessJson f);

    unsigned short getPort();

    asio::ip::tcp getAddress();

   private:
    void startAccept();

    asio::ip::tcp::acceptor acceptor_;
    asio::ip::tcp address_;
    unsigned short port_;
    ProcessJson processJson_;
};

#endif  // SendReceive_H