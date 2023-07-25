#include "SenderReceivers.h"

#include <algorithm>
#include <boost/asio.hpp>
#include <boost/asio/read.hpp>
#include <functional>
#include <future>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;
using ProcessJson = std::function<std::string(const json &)>;

MulticasterSender::MulticasterSender(asio::io_context &ioContext,
                                     const std::string multicastAddress,
                                     unsigned short multicastPort)
    : socket_(ioContext),
      multicastEndpoint_(asio::ip::make_address(multicastAddress), multicastPort),
      multicastAddress_(multicastAddress), multicastPort_(multicastPort) {
    socket_.open(multicastEndpoint_.protocol());
    socket_.set_option(asio::ip::udp::socket::reuse_address(true));
}

void
MulticasterSender::send(const json &jsonData) {
    std::string jsonString = jsonData.dump();
    socket_.send_to(asio::buffer(jsonString), multicastEndpoint_);
}

unsigned short
MulticasterSender::getPort() {
    return multicastPort_;
}

std::string
MulticasterSender::getAddress() {
    return multicastAddress_;
}

MulticasterReceiver::MulticasterReceiver(asio::io_context &ioContext,
                                         const std::string multicastAddress,
                                         unsigned short multicastPort, ProcessJson processJson)
    : socket_(ioContext),
      multicastEndpoint_(asio::ip::make_address(multicastAddress), multicastPort),
      multicastAddress_(multicastAddress), multicastPort_(multicastPort),
      processJson_(std::move(processJson)) {
    socket_.open(multicastEndpoint_.protocol());
    socket_.set_option(asio::ip::udp::socket::reuse_address(true));
    socket_.bind(multicastEndpoint_);
    socket_.set_option(asio::ip::multicast::join_group(multicastEndpoint_.address()));
}

void
MulticasterReceiver::receive() {
    std::array<char, 1024> buffer;
    asio::ip::udp::endpoint senderEndpoint;

    size_t bytesRead = socket_.receive_from(asio::buffer(buffer), senderEndpoint);

    std::string message(buffer.data(), bytesRead);
    try {
        json jsonData = nlohmann::json::parse(message);
        // Process
        std::cout << "Message received from " << senderEndpoint.address().to_string() << ": "
                  << jsonData << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return;
    }
}

TCPClient::TCPClient(asio::io_context &ioContext, const std::string serverAddress,
                     const std::string port)
    : ioContext_(ioContext), socket_(ioContext), serverAddress_(serverAddress), port_(port) {
    asio::ip::tcp::resolver resolver(ioContext);
    auto endpoints = resolver.resolve(serverAddress, port);

    asio::async_connect(
        socket_, endpoints,
        [this](const boost::system::error_code &error, const asio::ip::tcp::endpoint &) {
            if (!error) {
                std::cout << "Connected to server" << std::endl;
            } else {
                std::cerr << "Error connecting to server: " << error.message() << std::endl;
            }
        });
}

json
TCPClient::send(const json &jsonData) {
    std::promise<std::string> promise;
    std::future<std::string> future = promise.get_future();

    std::string jsonString = jsonData.dump();
    asio::async_write(socket_, asio::buffer(jsonString + "\n"),
                      [this, jsonString, &promise](const boost::system::error_code &error,
                                                   std::size_t /*bytesTransferred*/) {
                          if (!error) {
                              readMessage(std::move(promise));
                          } else {
                              std::cerr << "Error sending data: " << error.message() << std::endl;
                              promise.set_exception(std::make_exception_ptr(
                                  std::runtime_error("Error sending data")));
                          }
                      });
    return json::parse(future.get());
}

void
TCPClient::readMessage(std::promise<std::string> promise) {
    asio::streambuf responseBuffer;
    asio::async_read_until(
        socket_, responseBuffer, '\n',
        [this, promise = std::move(promise), &responseBuffer](
            const boost::system::error_code &error, std::size_t /*bytes_transferred*/) mutable {
            if (!error) {
                std::istream responseStream(&responseBuffer);
                std::string message;
                std::getline(responseStream, message);
                std::cout << "Received message: " << message << std::endl;
                promise.set_value(message);
            } else {
                std::cerr << "Error receiving order ID: " << error.message() << std::endl;
                promise.set_exception(
                    std::make_exception_ptr(std::runtime_error("Error receiving order ID")));
            }

            this->ioContext_.stop();
        });
}

TCPSession::TCPSession(asio::ip::tcp::socket socket, ProcessJson f)
    : socket_(std::move(socket)), processJson_(std::move(f)) {}

void
TCPSession::start() {
    auto self(shared_from_this());
    asio::async_read_until(
        socket_, buffer_, '\n',
        [this, self](const boost::system::error_code &error, std::size_t bytesTransferred) {
            if (!error) {
                std::string message(asio::buffers_begin(buffer_.data()),
                                    asio::buffers_begin(buffer_.data()) + bytesTransferred);
                try {
                    json jsonData = nlohmann::json::parse(message);
                    json return_message;
                    return_message["message"] = this->processJson_(jsonData) + "\n";
                    std::cout << "Received from client: " << jsonData;
                    this->send(return_message.dump());
                } catch (const std::exception &e) {
                    std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                    return;
                }

                // Processor jsonData
            }
        });
}

void
TCPSession::send(const json &jsonData) {
    std::string jsonString = jsonData.dump();
    asio::async_write(socket_, asio::buffer(jsonString + "\n"),
                      [this, jsonString](const boost::system::error_code &error,
                                         std::size_t /*bytesTransferred*/) {
                          if (!error) {
                              std::cout << "Sent to client: " << jsonString << std::endl;
                          } else {
                              std::cerr << "Error sending data: " << error.message() << std::endl;
                          }
                      });
}

TCPServer::TCPServer(asio::io_context &ioContext, asio::ip::tcp address, unsigned short port,
                     ProcessJson f)
    : acceptor_(ioContext, asio::ip::tcp::endpoint(address, port)), address_(address), port_(port),
      processJson_(std::move(f)) {
    std::cout << "Server listening on port " << port << std::endl;
    startAccept();
};

TCPServer::TCPServer(asio::io_context &ioContext, unsigned short port, ProcessJson f)
    : acceptor_(ioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      address_(asio::ip::tcp::v4()), port_(port), processJson_(std::move(f)) {
    std::cout << "Server listening on port " << port << std::endl;
    startAccept();
}

unsigned short
TCPServer::getPort() {
    return port_;
}

asio::ip::tcp
TCPServer::getAddress() {
    return address_;
}

void
TCPServer::startAccept() {
    acceptor_.async_accept(
        [this](const boost::system::error_code &error, asio::ip::tcp::socket socket) {
            if (!error) {
                std::cout << "New client connected" << std::endl;
                std::make_shared<TCPSession>(
                    std::move(socket),
                    [this](const json &order) -> std::string { return this->processJson_(order); })
                    ->start();
            }
            startAccept(); // Accept the next connection
        });
}
