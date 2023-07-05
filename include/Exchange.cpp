#include "Exchange.h"

#include <boost/asio.hpp>
#include <chrono>
#include <nlohmann/json.hpp>
#include <string>

#include "Data.h"
#include "LimitOrderBook.h"

namespace asio = boost::asio;

using json = nlohmann::json;

Exchange::Exchange(asio::io_context& ioContext,
                   const std::string multicastAddress,
                   unsigned short multicastPort, asio::ip::tcp tcpAddress,
                   unsigned short tcpPort)
    : dataBroadcaster(ioContext, multicastAddress, multicastPort),
      tcpServer(ioContext, tcpAddress, tcpPort, [this](const json& jsonData) {
          this->processJsonOrder(jsonData);
      }) {
    this->nextOid = 0;
}

Exchange::~Exchange() {}

void Exchange::addSymbol(std::string symbol, double max_price,
                         double tick_size) {
    this->books.push_back(LimitOrderBook(*this, symbol, max_price, tick_size));
}

void Exchange::addOrder(std::string symbol, Dir direction, double price,
                        int volume, int cid) {
    std::time_t timestamp =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    Order order = {direction, price, volume, timestamp, this->nextOid++, cid};
    this->orderToBook[order.oid] = this->getBook(symbol);
    return this->getBook(symbol)->addOrder(order);
}

// I REALLY DON'T LIKE THIS METHOD FORWARDING BUT HOW ELSE DO YOU DO IT
void Exchange::cancelOrder(std::string symbol, int oid) {
    this->orderToBook[oid]->cancelOrder(oid);
}

void Exchange::editOrder(std::string symbol, int oid, int volume) {
    this->orderToBook[oid]->editOrder(oid, volume);
}

void Exchange::printOrderBook(std::string symbol) {
    this->getBook(symbol)->printOrderBook();
}

double Exchange::getBestBid(std::string symbol) {
    return this->getBook(symbol)->getBestBid();
}

double Exchange::getBestAsk(std::string symbol) {
    return this->getBook(symbol)->getBestAsk();
}

Order Exchange::getOrder(int oid) {
    return this->orderToBook[oid]->getOrder(oid);
}

// Wrap this in an optional otherwise the code will error and crash
LimitOrderBook* Exchange::getBook(std::string symbol) {
    for (LimitOrderBook& book : this->books) {
        if (book.getSymbol() == symbol) {
            return &book;
        }
    }
    return nullptr;
}

void printTradeLog(TradeLog trade_log) {
    std::visit(
        [](const auto& log) {
            std::cout << "TradeLog";
            if constexpr (std::is_same_v<std::decay_t<decltype(log)>, ADD>) {
                std::cout << "Type: ADD, "
                          << "Direction: "
                          << (log.direction == Dir::BUY ? "BUY" : "SELL")
                          << ", "
                          << "Price: " << log.price << ", "
                          << "Volume: " << log.volume << ", "
                          << "Order ID: " << log.oid << ", "
                          << "Timestamp: "
                          << std::put_time(std::localtime(&log.timestamp),
                                           "%F %T");
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>,
                                                CANCEL>) {
                std::cout << "Type: CANCEL, "
                          << "Direction: "
                          << (log.direction == Dir::BUY ? "BUY" : "SELL")
                          << ", "
                          << "Price: " << log.price << ", "
                          << "Volume: " << log.volume << ", "
                          << "Order ID: " << log.oid << ", "
                          << "Timestamp: "
                          << std::put_time(std::localtime(&log.timestamp),
                                           "%F %T");
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>,
                                                TRADE>) {
                std::cout << "Type: TRADE, "
                          << "Price: " << log.price << ", "
                          << "Volume: " << log.volume << ", "
                          << "Timestamp: "
                          << std::put_time(std::localtime(&log.timestamp),
                                           "%F %T")
                          << ", "
                          << "Aggressor: "
                          << (log.aggressor == Dir::BUY ? "BUY" : "SELL")
                          << ", "
                          << "Bid Order ID: " << log.bid_oid << ", "
                          << "Ask Order ID: " << log.ask_oid;
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>,
                                                EDIT>) {
                std::cout << "Type: EDIT, "
                          << "Direction: "
                          << (log.direction == Dir::BUY ? "BUY" : "SELL")
                          << ", "
                          << "Price: " << log.price << ", "
                          << "Old Volume: " << log.old_volume << ", "
                          << "New Volume: " << log.new_volume << ", "
                          << "Order ID: " << log.oid << ", "
                          << "Timestamp: "
                          << std::put_time(std::localtime(&log.timestamp),
                                           "%F %T");
            }
            std::cout << std::endl;
        },
        trade_log);
}

void Exchange::broadcast(TradeLog log) {
    printTradeLog(log);
    this->dataBroadcaster.send(serializeTradeLog(log));
    this->logs.push(log);
}

void Exchange::processJsonOrder(const json& jsonOrder) {
    std::string type = jsonOrder["type"];

    if (type == "ADD") {
        this->processJsonAdd(jsonOrder);
    } else if (type == "CANCEL") {
        this->processJsonCancel(jsonOrder);
    } else if (type == "EDIT") {
        this->processJsonEdit(jsonOrder);
    }
}

void Exchange::processJsonAdd(json jsonOrder) {
    std::string symbol = jsonOrder["symbol"];
    Dir direction = jsonOrder["direction"] == "BUY" ? Dir::BUY : Dir::SELL;
    double price = jsonOrder["price"];
    int volume = jsonOrder["volume"];
    int cid = jsonOrder["cid"];

    this->addOrder(symbol, direction, price, volume, cid);
}

void Exchange::processJsonCancel(json jsonOrder) {
    std::string symbol = jsonOrder["symbol"];
    int oid = jsonOrder["oid"];

    this->cancelOrder(symbol, oid);
}

void Exchange::processJsonEdit(json jsonOrder) {
    std::string symbol = jsonOrder["symbol"];
    int oid = jsonOrder["oid"];
    int volume = jsonOrder["volume"];

    this->editOrder(symbol, oid, volume);
}
