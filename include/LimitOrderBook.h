#ifndef LIMITORDERBOOK_H
#define LIMITORDERBOOK_H

#include <chrono>
#include <deque>
#include <queue>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "Data.h"

class Exchange;

// Define the Limit class
class Limit {
   public:
    Limit();
    Limit(double price);
    double getPrice();
    int getTotalVolume();
    void addOrder(Order order);
    void removeOrder(int oid);
    Order getOrder(int oid);
    Order getPriorityOrder();
    void updateOrder(int oid, int volume);

   private:
    std::deque<Order> orders;
    double price;
    int totalVolume;
};

// Define the LimitOrderBook class
class LimitOrderBook {
   public:
    LimitOrderBook(Exchange& exchange, std::string symbol, double max_price,
                   double tick);
    void addOrder(Order order);
    void cancelOrder(int oid);
    void editOrder(int oid, int volume);
    Order getOrder(int oid);
    std::string getSymbol();
    double getBestBid();
    double getBestAsk();
    void printOrderBook();
    void addLog(TradeLog log);

   private:
    std::vector<Limit> bids;
    std::vector<Limit> asks;
    std::unordered_map<int, Limit*> orderIDs;

    Exchange& exchange;
    std::string symbol;
    double tick;
    double bestBid;
    double bestAsk;
    double maxPrice;

    void processTrade(Order* bid, Order* ask, Dir aggressor);
    void matchBidOrder(Order* order);
    void matchAskOrder(Order* order);
    void deleteOrder(int oid);
};

#endif  // LIMITORDERBOOK_H
