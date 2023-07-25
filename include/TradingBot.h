#ifndef TRADINGBOT_H
#define TRADINGBOT_H

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "Data.h"
#include "LimitOrderBook.h"
#include "SenderReceivers.h"

using json = nlohmann::json;
namespace asio = boost::asio;

struct SymbolTradingInfo {
    double bestBid;
    double bestAsk;
    int bestBidVolume;
    int bestAskVolume;
};

class TradingBot {
  public:
    TradingBot(std::string id, std::vector<std::string> symbols, asio::io_context &ioContext,
               const std::string marketDataAddress, unsigned short marketDataPort,
               const std::string marketInfoddress, unsigned short marketInfoPort,
               const std::string tcpAddress, const std::string tcpPort);
    virtual void processMarketData(const json &jsonData);
    virtual void processMarketInfo(const json &jsonData);
    void addOrder(Dir dir, std::string symbol, int price, int volume);
    void cancelOrder(std::string symbol, int orderId);
    void editOrder(std::string symbol, int orderId, int newVolume);
    std::string getId();

  private:
    std::string id;
    LimitOrderBooks limitOrderBooks;
    TCPClient tcpClient;
    MulticasterReceiver marketDataReceiver;
    MulticasterReceiver marketInfoReceiver;
    std::unordered_map<std::string, double> symbolToInternalPrice;
};

class KnowsPriceBot : public TradingBot {
  public:
    using TradingBot::TradingBot;
    void processMarketInfo(const json &jsonData) override;
    void processMarketData(const json &jsonData) override;

  private:
    // I need a datastructure to track how many new trades have been made since last period;
};

class MarketMakerBot : public TradingBot {
  public:
    using TradingBot::TradingBot;
    void processMarketInfo(const json &jsonData) override;
    void processMarketData(const json &jsonData) override;
};
#endif // TRADINGBOT_H