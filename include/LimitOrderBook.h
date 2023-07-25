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

class LimitOrderBook;
class LimitOrderBooks;
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

class LimitOrderBooks {
  public:
    LimitOrderBooks(std::vector<std::string> symbols);
    void addSymbol(std::string symbol, double max_price, double tick_size);
    int addOrder(std::string symbol, Dir direction, double price, int volume, int cid);
    void cancelOrder(std::string symbol, int oid);
    void editOrder(std::string symbol, int oid, int volume);
    void printOrderBook(std::string symbol);
    double getBestBid(std::string symbol);
    double getBestAsk(std::string symbol);
    Order getOrder(int oid);
    virtual void addLog(TradeLog log);
    std::string processJsonOrder(const json &jsonOrder);

  private:
    std::vector<LimitOrderBook> books;
    std::queue<TradeLog> logs;
    std::unordered_map<int, LimitOrderBook *> orderToBook;

    int nextOid;

    // Should this be in this class or should I create a new parser class?
    LimitOrderBook *getBook(std::string symbol);
};

// Define the LimitOrderBook class
class LimitOrderBook {
  public:
    LimitOrderBook(LimitOrderBooks &limitOrderBooks, std::string symbol, double max_price,
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
    std::unordered_map<int, Limit *> oidsToLimit;

    LimitOrderBooks &limitOrderBooks;
    std::string symbol;
    double tick;
    double bestBid;
    double bestAsk;
    double maxPrice;

    void processTrade(Order *bid, Order *ask, Dir aggressor);
    void matchBidOrder(Order *order);
    void matchAskOrder(Order *order);
    void deleteOrder(int oid);
};

#endif   // LIMITORDERBOOK_H
