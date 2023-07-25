#include "Information.h"

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <random>
#include "SenderReceivers.h"
#include <chrono>
#include <vector>

using json = nlohmann::json;

InfoBroadcaster::InfoBroadcaster(std::vector<std::string> symbols, asio::io_context &ioContext,
                                 const std::string multicastAddress, unsigned short multicastPort)
    : symbols(symbols), dataBroadcaster(ioContext, multicastAddress, multicastPort) {}

void
InfoBroadcaster::broadcastMarketInfo(MarketInfo marketInfo) {
    json jsonData = serializeMarketInfo(marketInfo);
    dataBroadcaster.send(jsonData.dump());
}

TimeBroadcaster::TimeBroadcaster(std::vector<std::string> symbols, asio::io_context &ioContext,
                                 const std::string multicastAddress, unsigned short multicastPort,
                                 int remainingMilliseconds)
    : InfoBroadcaster(symbols, ioContext, multicastAddress, multicastPort),
      remainingMilliseconds(remainingMilliseconds) {}

void
TimeBroadcaster::run() {
    std::random_device rd; // Obtain a random seed from the hardware
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        this->remainingMilliseconds -= 100;
        RemainingMilliseconds remainingMilliseconds = {this->remainingMilliseconds};
        this->broadcastMarketInfo(remainingMilliseconds);
    }
}