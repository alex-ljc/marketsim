#ifndef TRADINGBOT_H
#define TRADINGBOT_H

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <vector>

#include "Data.h"
#include "SenderReceivers.h"

using json = nlohmann::json;
namespace asio = boost::asio;

class TradingBot {
   public:
    TradingBot(asio::io_context& ioContext, const std::string marketDataAddress,
               unsigned short marketDataPort,
               const std::string marketInfoddress,
               unsigned short marketInfoPort, const std::string tcpAddress,
               const std::string tcpPort);
    void sendOrder();
    void editOrder();
    void cancelOrder();
    virtual void processMarketData(const json& jsonData);
    virtual void processMarketInfo(const json& jsonData);

   private:
    TCPClient tcpClient;
    MulticasterReceiver marketDataReceiver;
    MulticasterReceiver marketInfoReceiver;
    std::vector<Order> sentOrders;
};

#endif  // TRADINGBOT_H