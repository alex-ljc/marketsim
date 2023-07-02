#ifndef LIMITORDERBOOK_H
#define LIMITORDERBOOK_H

#include <chrono>
#include <cmath>
#include <deque>
#include <iomanip>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

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
    LimitOrderBook(double max_price, double tick);
    void addOrder(Dir direction, double price, int volume, int cid);
    void cancelOrder(int oid);
    Order getOrder(int oid);
    double getBestBid();
    double getBestAsk();
    void printOrderBook();

   private:
    std::vector<Limit> bids;
    std::vector<Limit> asks;
    std::unordered_map<int, Limit*> orderIDs;
    int nextOID;
    double tick;
    double bestBid;
    double bestAsk;
    double maxPrice;

    void matchBidOrder(Order* order);
    void matchAskOrder(Order* order);
    void deleteOrder(int oid);
};

#endif  // LIMITORDERBOOK_H
