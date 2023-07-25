#include "Exchange.h"

#include <boost/asio.hpp>
#include <chrono>
#include <nlohmann/json.hpp>
#include <string>

#include "Data.h"
#include "LimitOrderBook.h"

namespace asio = boost::asio;

using json = nlohmann::json;

Exchange::Exchange(std::vector<std::string> symbols, asio::io_context &ioContext,
                   const std::string multicastAddress, unsigned short multicastPort,
                   asio::ip::tcp tcpAddress, unsigned short tcpPort)
    : LimitOrderBooks(symbols), dataBroadcaster(ioContext, multicastAddress, multicastPort),
      tcpServer(ioContext, tcpAddress, tcpPort, [this](const json &order) -> std::string {
          return this->processJsonOrder(order);
      }) {}

Exchange::~Exchange() {}

void
Exchange::addLog(TradeLog log) {
    this->LimitOrderBooks::addLog(log);
    printTradeLog(log);
    this->dataBroadcaster.send(serializeTradeLog(log));
}
