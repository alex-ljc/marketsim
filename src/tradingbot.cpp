
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "TradingBot.h"

namespace asio = boost::asio;

int
main() {
    asio::io_context io_context;
    TradingBot tradingBot("test", {"AAPL"}, io_context, "239.255.0.1", 5000, "", 5050, "127.0.0.1",
                          "8080");
}
