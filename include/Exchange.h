#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <nlohmann/json.hpp>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "Data.h"
#include "SenderReceivers.h"

using json = nlohmann::json;

class LimitOrderBook;

class Exchange {
   public:
    Exchange(asio::io_context &ioContext, const std::string multicastAddress,
             unsigned short multicastPort, asio::ip::tcp tcpAddress,
             unsigned short tcpPort);
    ~Exchange();
    void addSymbol(std::string symbol, double max_price, double tick_size);
    void addOrder(std::string symbol, Dir direction, double price, int volume,
                  int cid);
    void cancelOrder(std::string symbol, int oid);
    void editOrder(std::string symbol, int oid, int volume);
    void printOrderBook(std::string symbol);
    double getBestBid(std::string symbol);
    double getBestAsk(std::string symbol);
    Order getOrder(int oid);
    void broadcast(TradeLog log);
    // I don't like the naming of this function here. It's meant to take in
    // json from the tcp client and then edit the order books accordingly
    void processJsonOrder(const json &jsonOrder);

   private:
    std::vector<LimitOrderBook> books;
    std::queue<TradeLog> logs;
    std::unordered_map<int, LimitOrderBook *> orderToBook;
    MulticasterSender dataBroadcaster;
    TCPServer tcpServer;

    int nextOid;

    // Should this be in this class or should I create a new parser class?
    LimitOrderBook *getBook(std::string symbol);
    void processJsonAdd(json jsonOrder);
    void processJsonCancel(json jsonCancel);
    void processJsonEdit(json jsonEdit);
};

#endif  // EXCHANGE_H