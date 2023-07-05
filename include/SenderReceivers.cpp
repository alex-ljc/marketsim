#include "SenderReceivers.h"

#include <boost/asio.hpp>
#include <functional>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;
using ProcessJson = std::function<void(const json&)>;

MulticasterSender::MulticasterSender(asio::io_context& ioContext,
                                     const std::string multicastAddress,
                                     unsigned short multicastPort)
    : socket_(ioContext),
      multicastEndpoint_(asio::ip::make_address(multicastAddress),
                         multicastPort),
      multicastAddress_(multicastAddress),
      multicastPort_(multicastPort) {
    socket_.open(multicastEndpoint_.protocol());
    socket_.set_option(asio::ip::udp::socket::reuse_address(true));
}

void MulticasterSender::send(const json& jsonData) {
    std::string jsonString = jsonData.dump();
    socket_.send_to(asio::buffer(jsonString), multicastEndpoint_);
}

unsigned short MulticasterSender::getPort() { return multicastPort_; }

std::string MulticasterSender::getAddress() { return multicastAddress_; }

MulticasterReceiver::MulticasterReceiver(asio::io_context& ioContext,
                                         const std::string multicastAddress,
                                         unsigned short multicastPort,
                                         ProcessJson processJson)
    : socket_(ioContext),
      multicastEndpoint_(asio::ip::make_address(multicastAddress),
                         multicastPort),
      multicastAddress_(multicastAddress),
      multicastPort_(multicastPort),
      processJson_(std::move(processJson)) {
    socket_.open(multicastEndpoint_.protocol());
    socket_.set_option(asio::ip::udp::socket::reuse_address(true));
    socket_.bind(multicastEndpoint_);
    socket_.set_option(
        asio::ip::multicast::join_group(multicastEndpoint_.address()));
}

void MulticasterReceiver::receive() {
    std::array<char, 1024> buffer;
    asio::ip::udp::endpoint senderEndpoint;

    size_t bytesRead =
        socket_.receive_from(asio::buffer(buffer), senderEndpoint);

    std::string message(buffer.data(), bytesRead);
    try {
        json jsonData = nlohmann::json::parse(message);
        // Process
        std::cout << "Message received from "
                  << senderEndpoint.address().to_string() << ": " << jsonData
                  << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return;
    }
}

TCPClient::TCPClient(asio::io_context& ioContext,
                     const std::string serverAddress, const std::string port)
    : socket_(ioContext), serverAddress_(serverAddress), port_(port) {
    asio::ip::tcp::resolver resolver(ioContext);
    auto endpoints = resolver.resolve(serverAddress, port);

    asio::async_connect(socket_, endpoints,
                        [this](const boost::system::error_code& error,
                               const asio::ip::tcp::endpoint&) {
                            if (!error) {
                                std::cout << "Connected to server" << std::endl;
                            } else {
                                std::cerr << "Error connecting to server: "
                                          << error.message() << std::endl;
                            }
                        });
}

void TCPClient::send(const json& jsonData) {
    std::string jsonString = jsonData.dump();
    asio::async_write(
        socket_, asio::buffer(jsonString + "\n"),
        [this, jsonString](const boost::system::error_code& error,
                           std::size_t /*bytesTransferred*/) {
            if (!error) {
                std::cout << "Sent to server: " << jsonString << std::endl;
            } else {
                std::cerr << "Error sending data: " << error.message()
                          << std::endl;
            }
        });
}

TCPSession::TCPSession(asio::ip::tcp::socket socket, ProcessJson f)
    : socket_(std::move(socket)), processJson_(std::move(f)) {}

void TCPSession::start() {
    auto self(shared_from_this());
    asio::async_read_until(
        socket_, buffer_, '\n',
        [this, self](const boost::system::error_code& error,
                     std::size_t bytesTransferred) {
            if (!error) {
                std::string message(
                    asio::buffers_begin(buffer_.data()),
                    asio::buffers_begin(buffer_.data()) + bytesTransferred);
                try {
                    json jsonData = nlohmann::json::parse(message);
                    this->processJson_(jsonData);
                    std::cout << "Received from client: " << jsonData;
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing JSON: " << e.what()
                              << std::endl;
                    return;
                }

                // Processor jsonData
            }
        });
}

TCPServer::TCPServer(asio::io_context& ioContext, asio::ip::tcp address,
                     unsigned short port, ProcessJson f)
    : acceptor_(ioContext, asio::ip::tcp::endpoint(address, port)),
      address_(address),
      port_(port),
      processJson_(std::move(f)) {
    std::cout << "Server listening on port " << port << std::endl;
    startAccept();
};

TCPServer::TCPServer(asio::io_context& ioContext, unsigned short port,
                     ProcessJson f)
    : acceptor_(ioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      address_(asio::ip::tcp::v4()),
      port_(port),
      processJson_(std::move(f)) {
    std::cout << "Server listening on port " << port << std::endl;
    startAccept();
}

unsigned short TCPServer::getPort() { return port_; }

asio::ip::tcp TCPServer::getAddress() { return address_; }

void TCPServer::startAccept() {
    acceptor_.async_accept([this](const boost::system::error_code& error,
                                  asio::ip::tcp::socket socket) {
        if (!error) {
            std::cout << "New client connected" << std::endl;
            std::make_shared<TCPSession>(
                std::move(socket),
                [this](const json& order) { this->processJson_(order); })
                ->start();
        }
        startAccept();  // Accept the next connection
    });
}
