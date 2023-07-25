#include "Exchange.h"

#include <boost/asio.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "SenderReceivers.h"

namespace asio = boost::asio;

int main() {
    asio::io_context io_context;
    Exchange exchange(std::vector<std::string>{}, io_context, "239.255.0.1", 5000,
                      asio::ip::tcp::v4(), 8080);
    exchange.addSymbol("AAPL", 1000, 0.01);
    exchange.addOrder("AAPL", Dir::BUY, 100, 100, 0);
    exchange.addOrder("AAPL", Dir::SELL, 102, 100, 1);
    exchange.addOrder("AAPL", Dir::SELL, 101, 100, 2);
    exchange.addOrder("AAPL", Dir::BUY, 99, 100, 3);
    exchange.addOrder("AAPL", Dir::BUY, 101, 100, 3);
    exchange.editOrder("AAPL", 1, 50);
    exchange.printOrderBook("AAPL");

    return 0;
}
