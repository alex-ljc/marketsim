#include "TradingBot.h"

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <variant>
#include <vector>

#include "Data.h"
#include "Information.h"
#include "LimitOrderBook.h"
#include "SenderReceivers.h"

TradingBot::TradingBot(std::string id, std::vector<std::string> symbols,
                       asio::io_context &ioContext, const std::string marketDataAddress,
                       unsigned short marketDataPort, const std::string marketInfoddress,
                       unsigned short marketInfoPort, const std::string tcpAddress,
                       const std::string tcpPort)
    : id(id), limitOrderBooks(symbols), tcpClient(ioContext, tcpAddress, tcpPort),
      marketDataReceiver(ioContext, marketDataAddress, marketDataPort,
                         [this](const json &jsonData) { this->processMarketData(jsonData); }),
      marketInfoReceiver(ioContext, marketInfoddress, marketInfoPort,
                         [this](const json &jsonData) { this->processMarketInfo(jsonData); }) {}

void
TradingBot::addOrder(Dir dir, std::string symbol, int price, int volume) {
    json order = {{"type", "ADD"},    {"dir", dir == Dir::BUY ? "BUY" : "SELL"},
                  {"symbol", symbol}, {"price", price},
                  {"volume", volume}, {"cid", this->getId()}};
    int oid = std::stoi(std::string(tcpClient.send(order)["message"]));
}

void
TradingBot::cancelOrder(std::string symbol, int orderId) {
    json cancel = {
        {"type", "CANCEL"}, {"symbol", symbol}, {"oid", orderId}, {"cid", this->getId()}};
    tcpClient.send(cancel);
}

void
TradingBot::editOrder(std::string symbol, int orderId, int newVolume) {
    json edit = {{"type", "EDIT"},
                 {"symbol", symbol},
                 {"orderId", orderId},
                 {"newVolume", newVolume},
                 {"cid", this->getId()}};
    tcpClient.send(edit);
}

void
TradingBot::processMarketData(const json &jsonData) {
    TradeLog trade_log = deserializeTradeLog(jsonData);
    std::visit(
        [&](const auto &log) {
            if constexpr (std::is_same_v<std::decay_t<decltype(log)>, ADD>) {
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>, CANCEL>) {
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>, TRADE>) {
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>, EDIT>) {
            }
        },
        trade_log);
}

void
TradingBot::processMarketInfo(const json &jsonData) {
    MarketInfo market_info = deserializeMarketInfo(jsonData);
    std::visit(
        [&](const auto &info) {
            if constexpr (std::is_same_v<std::decay_t<decltype(info)>, Prices>) {
                this->symbolToInternalPrice[info.symbol] = info.idToPrice.at(this->id);
                // Bot reacts to new price. For the current bot it shouldn't
                // need to
            } else if constexpr (std::is_same_v<std::decay_t<decltype(info)>,
                                                RemainingMilliseconds>) {
            }
        },
        market_info);
}

std::string
TradingBot::getId() {
    return this->id;
}

// The bot should trade on a timer. It should expect to make a certain amount of trades
// every second? If these trades aren't made it will accept shitter prices. If there
// are no prices on the market it'll also post it on prices.
void
KnowsPriceBot::processMarketData(const json &jsonData) {
    TradeLog trade_log = deserializeTradeLog(jsonData);
    std::visit(
        [&](const auto &log) {
            if constexpr (std::is_same_v<std::decay_t<decltype(log)>, ADD>) {

            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>, CANCEL>) {

            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>, TRADE>) {

            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>, EDIT>) {
            }
        },
        trade_log);
}
