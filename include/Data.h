#ifndef DATA_H
#define DATA_H

#include "Information.h"
#include <chrono>
#include <iomanip> // For std::put_time and formatting options
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <variant>

enum class Dir { BUY, SELL };

// Define the Order struct
struct Order {
    Dir direction;
    double price;
    int volume;
    std::time_t timestamp;
    int oid;
    int cid;
    std::string symbol;
};

struct ADD {
    Dir direction;
    double price;
    int volume;
    int oid;
    std::time_t timestamp;
    std::string symbol;
};

struct CANCEL {
    Dir direction;
    double price;
    int volume;
    int oid;
    std::time_t timestamp;
    std::string symbol;
};

struct TRADE {
    double price;
    int volume;
    std::time_t timestamp;
    Dir aggressor;
    int bid_oid;
    int ask_oid;
    std::string symbol;
};

struct EDIT {
    Dir direction;
    double price;
    int old_volume;
    int new_volume;
    int oid;
    std::time_t timestamp;
    std::string symbol;
};

using TradeLog = std::variant<ADD, CANCEL, TRADE, EDIT>;

using json = nlohmann::json;

inline json
serializeTradeLog(const TradeLog &trade_log) {
    json tradeLogJson;

    std::visit(
        [&](const auto &log) {
            if constexpr (std::is_same_v<std::decay_t<decltype(log)>, ADD>) {
                tradeLogJson["Type"] = "ADD";
                tradeLogJson["Direction"] = (log.direction == Dir::BUY) ? "BUY" : "SELL";
                tradeLogJson["Price"] = log.price;
                tradeLogJson["Volume"] = log.volume;
                tradeLogJson["Order ID"] = log.oid;

                std::stringstream ss;
                ss << std::put_time(std::localtime(&log.timestamp), "%F %T");
                tradeLogJson["Timestamp"] = ss.str();
                tradeLogJson["Symbol"] = log.symbol;
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>, CANCEL>) {
                tradeLogJson["Type"] = "CANCEL";
                tradeLogJson["Direction"] = (log.direction == Dir::BUY) ? "BUY" : "SELL";
                tradeLogJson["Price"] = log.price;
                tradeLogJson["Volume"] = log.volume;
                tradeLogJson["Order ID"] = log.oid;

                std::stringstream ss;
                ss << std::put_time(std::localtime(&log.timestamp), "%F %T");
                tradeLogJson["Timestamp"] = ss.str();
                tradeLogJson["Symbol"] = log.symbol;
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>, TRADE>) {
                tradeLogJson["Type"] = "TRADE";
                tradeLogJson["Price"] = log.price;
                tradeLogJson["Volume"] = log.volume;

                std::stringstream ss;
                ss << std::put_time(std::localtime(&log.timestamp), "%F %T");
                tradeLogJson["Timestamp"] = ss.str();

                tradeLogJson["Aggressor"] = (log.aggressor == Dir::BUY) ? "BUY" : "SELL";
                tradeLogJson["Bid Order ID"] = log.bid_oid;
                tradeLogJson["Ask Order ID"] = log.ask_oid;
                tradeLogJson["Symbol"] = log.symbol;
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>, EDIT>) {
                tradeLogJson["Type"] = "EDIT";
                tradeLogJson["Direction"] = (log.direction == Dir::BUY) ? "BUY" : "SELL";
                tradeLogJson["Price"] = log.price;
                tradeLogJson["Old Volume"] = log.old_volume;
                tradeLogJson["New Volume"] = log.new_volume;
                tradeLogJson["Order ID"] = log.oid;

                std::stringstream ss;
                ss << std::put_time(std::localtime(&log.timestamp), "%F %T");
                tradeLogJson["Timestamp"] = ss.str();
                tradeLogJson["Symbol"] = log.symbol;
            }
        },
        trade_log);

    return tradeLogJson;
}

inline TradeLog
deserializeTradeLog(const json &tradeLogJson) {
    std::string type = tradeLogJson["Type"];
    if (type == "ADD") {
        Dir direction = (tradeLogJson["Direction"] == "BUY") ? Dir::BUY : Dir::SELL;
        double price = tradeLogJson["Price"];
        int volume = tradeLogJson["Volume"];
        int oid = tradeLogJson["Order ID"];
        std::time_t timestamp = tradeLogJson["Timestamp"];
        std::string symbol = tradeLogJson["Symbol"];
        return ADD{direction, price, volume, oid, timestamp, symbol};
    } else if (type == "CANCEL") {
        Dir direction = (tradeLogJson["Direction"] == "BUY") ? Dir::BUY : Dir::SELL;
        double price = tradeLogJson["Price"];
        int volume = tradeLogJson["Volume"];
        int oid = tradeLogJson["Order ID"];
        std::time_t timestamp = tradeLogJson["Timestamp"];
        std::string symbol = tradeLogJson["Symbol"];
        return CANCEL{direction, price, volume, oid, timestamp, symbol};
    } else if (type == "TRADE") {
        double price = tradeLogJson["Price"];
        int volume = tradeLogJson["Volume"];
        std::time_t timestamp = tradeLogJson["Timestamp"];
        Dir aggressor = (tradeLogJson["Aggressor"] == "BUY") ? Dir::BUY : Dir::SELL;
        int bid_oid = tradeLogJson["Bid Order ID"];
        int ask_oid = tradeLogJson["Ask Order ID"];
        std::string symbol = tradeLogJson["Symbol"];
        return TRADE{price, volume, timestamp, aggressor, bid_oid, ask_oid, symbol};
    } else {
        Dir direction = (tradeLogJson["Direction"] == "BUY") ? Dir::BUY : Dir::SELL;
        double price = tradeLogJson["Price"];
        int old_volume = tradeLogJson["Old Volume"];
        int new_volume = tradeLogJson["New Volume"];
        int oid = tradeLogJson["Order ID"];
        std::time_t timestamp = tradeLogJson["Timestamp"];
        std::string symbol = tradeLogJson["Symbol"];
        return EDIT{direction, price, old_volume, new_volume, oid, timestamp, symbol};
    }
}

inline void
printTradeLog(TradeLog trade_log) {
    std::visit(
        [](const auto &log) {
            if constexpr (std::is_same_v<std::decay_t<decltype(log)>, ADD>) {
                std::cout << "Type: ADD, "
                          << "Direction: " << (log.direction == Dir::BUY ? "BUY" : "SELL") << ", "
                          << "Price: " << log.price << ", "
                          << "Volume: " << log.volume << ", "
                          << "Order ID: " << log.oid << ", "
                          << "Timestamp: "
                          << std::put_time(std::localtime(&log.timestamp), "%F %T");
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>, CANCEL>) {
                std::cout << "Type: CANCEL, "
                          << "Direction: " << (log.direction == Dir::BUY ? "BUY" : "SELL") << ", "
                          << "Price: " << log.price << ", "
                          << "Volume: " << log.volume << ", "
                          << "Order ID: " << log.oid << ", "
                          << "Timestamp: "
                          << std::put_time(std::localtime(&log.timestamp), "%F %T");
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>, TRADE>) {
                std::cout << "Type: TRADE, "
                          << "Aggressor: " << (log.aggressor == Dir::BUY ? "BUY" : "SELL") << ", "
                          << "Price: " << log.price << ", "
                          << "Volume: " << log.volume << ", "
                          << "Bid Order ID: " << log.bid_oid << ", "
                          << "Ask Order ID: " << log.ask_oid << ", "
                          << "Timestamp: "
                          << std::put_time(std::localtime(&log.timestamp), "%F %T");
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>, EDIT>) {
                std::cout << "Type: EDIT, "
                          << "Direction: " << (log.direction == Dir::BUY ? "BUY" : "SELL") << ", "
                          << "Price: " << log.price << ", "
                          << "Old Volume: " << log.old_volume << ", "
                          << "New Volume: " << log.new_volume << ", "
                          << "Order ID: " << log.oid << ", "
                          << "Timestamp: "
                          << std::put_time(std::localtime(&log.timestamp), "%F %T");
            }
            std::cout << std::endl;
        },
        trade_log);
}

#endif // DATA_H