#include "TradingBot.h"

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <string>

#include "Data.h"
#include "SenderReceivers.h"

TradingBot::TradingBot(asio::io_context& ioContext,
                       const std::string marketDataAddress,
                       unsigned short marketDataPort,
                       const std::string marketInfoddress,
                       unsigned short marketInfoPort,
                       const std::string tcpAddress, const std::string tcpPort)
    : tcpClient(ioContext, tcpAddress, tcpPort),
      marketDataReceiver(
          ioContext, marketDataAddress, marketDataPort,
          [this](const json& jsonData) { this->processMarketData(jsonData); }),
      marketInfoReceiver(
          ioContext, marketInfoddress, marketInfoPort,
          [this](const json& jsonData) { this->processMarketInfo(jsonData); }) {
}

void TradingBot::processMarketData(const json& jsonData) {
    std::string type = jsonData["type"];

    switch (type) {
        case "add":
            processJsonAdd(jsonOrder);
            break;
        case "cancel":
            processJsonCancel(jsonOrder);
            break;
        case "edit":
            processJsonEdit(jsonOrder);
            break;
        default:
            std::cout << "Invalid type: " << type << std::endl;
            break;
    }
}
