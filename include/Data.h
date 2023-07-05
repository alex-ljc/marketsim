#ifndef DATA_H
#define DATA_H

#include <chrono>
#include <iomanip>  // For std::put_time and formatting options
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
};

struct ADD {
    Dir direction;
    double price;
    int volume;
    int oid;
    std::time_t timestamp;
};

struct CANCEL {
    Dir direction;
    double price;
    int volume;
    int oid;
    std::time_t timestamp;
};

struct TRADE {
    double price;
    int volume;
    std::time_t timestamp;
    Dir aggressor;
    int bid_oid;
    int ask_oid;
};

struct EDIT {
    Dir direction;
    double price;
    int old_volume;
    int new_volume;
    int oid;
    std::time_t timestamp;
};

struct EMPTY {};

struct ERROR {
    std::string ErrorMsg;
};

using TradeLog = std::variant<ADD, CANCEL, TRADE, EDIT, EMPTY, ERROR>;

using json = nlohmann::json;

inline json serializeTradeLog(const TradeLog& trade_log) {
    json tradeLogJson;

    std::visit(
        [&](const auto& log) {
            if constexpr (std::is_same_v<std::decay_t<decltype(log)>, ADD>) {
                tradeLogJson["Type"] = "ADD";
                tradeLogJson["Direction"] =
                    (log.direction == Dir::BUY) ? "BUY" : "SELL";
                tradeLogJson["Price"] = log.price;
                tradeLogJson["Volume"] = log.volume;
                tradeLogJson["Order ID"] = log.oid;

                std::stringstream ss;
                ss << std::put_time(std::localtime(&log.timestamp), "%F %T");
                tradeLogJson["Timestamp"] = ss.str();
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>,
                                                CANCEL>) {
                tradeLogJson["Type"] = "CANCEL";
                tradeLogJson["Direction"] =
                    (log.direction == Dir::BUY) ? "BUY" : "SELL";
                tradeLogJson["Price"] = log.price;
                tradeLogJson["Volume"] = log.volume;
                tradeLogJson["Order ID"] = log.oid;

                std::stringstream ss;
                ss << std::put_time(std::localtime(&log.timestamp), "%F %T");
                tradeLogJson["Timestamp"] = ss.str();
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>,
                                                TRADE>) {
                tradeLogJson["Type"] = "TRADE";
                tradeLogJson["Price"] = log.price;
                tradeLogJson["Volume"] = log.volume;

                std::stringstream ss;
                ss << std::put_time(std::localtime(&log.timestamp), "%F %T");
                tradeLogJson["Timestamp"] = ss.str();

                tradeLogJson["Aggressor"] =
                    (log.aggressor == Dir::BUY) ? "BUY" : "SELL";
                tradeLogJson["Bid Order ID"] = log.bid_oid;
                tradeLogJson["Ask Order ID"] = log.ask_oid;
            } else if constexpr (std::is_same_v<std::decay_t<decltype(log)>,
                                                EDIT>) {
                tradeLogJson["Type"] = "EDIT";
                tradeLogJson["Direction"] =
                    (log.direction == Dir::BUY) ? "BUY" : "SELL";
                tradeLogJson["Price"] = log.price;
                tradeLogJson["Old Volume"] = log.old_volume;
                tradeLogJson["New Volume"] = log.new_volume;
                tradeLogJson["Order ID"] = log.oid;

                std::stringstream ss;
                ss << std::put_time(std::localtime(&log.timestamp), "%F %T");
                tradeLogJson["Timestamp"] = ss.str();
            }
        },
        trade_log);

    return tradeLogJson;
}

#endif  // DATA_H