#ifndef INFORMATION_H
#define INFORMATION_H

#include "SenderReceivers.h"

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

using json = nlohmann::json;

// Bit of an unecessary abstraction rn but I can see it being helpful
struct Prices {
    std::string symbol;
    std::unordered_map<std::string, double> idToPrice;
};

// What is the best way to represent this?
struct RemainingMilliseconds {
    double remainingMilliseconds;
};

using MarketInfo = std::variant<Prices, RemainingMilliseconds>;

inline json
serializeMarketInfo(MarketInfo marketInfo) {
    json jsonData;

    std::visit(
        [&](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Prices>) {
                jsonData["type"] = "prices";
                jsonData["symbol"] = arg.symbol;
                jsonData["prices"] = arg.idToPrice;
            } else if constexpr (std::is_same_v<T, RemainingMilliseconds>) {
                jsonData["type"] = "remaining_milliseconds";
                jsonData["remaining_milliseconds"] = arg.remainingMilliseconds;
            }
        },
        marketInfo);
    return jsonData;
}

inline MarketInfo
deserializeMarketInfo(const json &jsonMarketInfo) {
    std::string type = jsonMarketInfo["type"];
    if (type == "prices") {
        std::string symbol = jsonMarketInfo["symbol"];
        std::unordered_map<std::string, double> newPrices = jsonMarketInfo["prices"];
        Prices prices = {symbol, newPrices};
        return prices;
    } else {
        double remainingMilliseconds = jsonMarketInfo["remaining_milliseconds"];
        return RemainingMilliseconds{remainingMilliseconds};
    }
};

class InfoBroadcaster {
  public:
    InfoBroadcaster(std::vector<std::string> symbols, asio::io_context &ioContext,
                    const std::string multicastAddress, unsigned short multicastPort);
    void broadcastMarketInfo(MarketInfo marketInfo);
    virtual void run();

  private:
    std::vector<std::string> symbols;
    MulticasterSender dataBroadcaster;
};

class TimeBroadcaster : public InfoBroadcaster {
  public:
    TimeBroadcaster(std::vector<std::string> symbols, asio::io_context &ioContext,
                    const std::string multicastAddress, unsigned short multicastPort,
                    int remainingMilliseconds);
    void run() override;

  private:
    double remainingMilliseconds;
};

#endif // INFORMATION_H