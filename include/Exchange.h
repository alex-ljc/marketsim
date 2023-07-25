#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <nlohmann/json.hpp>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "Data.h"
#include "LimitOrderBook.h"
#include "SenderReceivers.h"

using json = nlohmann::json;

class LimitOrderBook;

class Exchange : public LimitOrderBooks {
  public:
    Exchange(std::vector<std::string> symbols, asio::io_context &ioContext,
             const std::string multicastAddress, unsigned short multicastPort,
             asio::ip::tcp tcpAddress, unsigned short tcpPort);
    ~Exchange();
    void addLog(TradeLog log) override;

  private:
    MulticasterSender dataBroadcaster;
    TCPServer tcpServer;
};

#endif   // EXCHANGE_H