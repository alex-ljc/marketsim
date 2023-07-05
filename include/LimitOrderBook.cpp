#include "LimitOrderBook.h"

#include <chrono>
#include <cmath>
#include <deque>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <optional>
#include <queue>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "Exchange.h"

bool isEqual(double a, double b) {
    if (std::fpclassify(a) != std::fpclassify(b)) {
        // Different classifications, not equal
        return false;
    }

    return std::fabs(a - b) < std::numeric_limits<double>::epsilon();
}

bool divisible(double a, double b) {
    int result;
#if 1
    if (fabsl(((roundl(a / b) * b) - a)) <= (1E-9 * b)) {
        result = true;
    } else {
        result = false;
    }
#else
    if (fabsl(remainderl(a, b)) <= (1E-9 * b)) {
        result = true;
    } else {
        result = false;
    }
#endif
    // printf("divisible(%Lg, %Lg): %Lg, %Lg,%d\n", a, b, roundl(a/b),
    return (result);
}

Limit::Limit() : orders(), price(0), totalVolume(0){};

Limit::Limit(double price) : orders(), price(price), totalVolume(0){};

double Limit::getPrice() { return this->price; };

int Limit::getTotalVolume() { return this->totalVolume; };

// Precondition: order.price == this->price
void Limit::addOrder(Order order) {
    if (!isEqual(order.price, this->price)) {
        std::cout << "Order has wrong price: " << order.price << std::endl;
        std::cout << "Limit price: " << this->price << std::endl;
        return;
    }

    this->price = order.price;
    this->totalVolume += order.volume;
    this->orders.push_back(order);
};
// Precondition: order is in the last position of this->orders and
// order.totalVolume += order.volume

// Precondition: order is in this->orders
void Limit::removeOrder(int oid) {
    for (auto it = this->orders.begin(); it != this->orders.end(); ++it) {
        if (it->oid == oid) {
            this->orders.erase(it);
            this->totalVolume -= it->volume;
            return;
        }
    }
}
// Postcondition: order is not in this->orders

Order Limit::getOrder(int oid) {
    for (auto it = this->orders.begin(); it != this->orders.end(); ++it) {
        if (it->oid == oid) {
            return *it;
        }
    }
    return Order();
}

Order Limit::getPriorityOrder() { return this->orders.front(); }

// Precondition: order is in this->orders
void Limit::updateOrder(int oid, int volume) {
    for (auto it = this->orders.begin(); it != this->orders.end(); ++it) {
        if (it->oid == oid) {
            if (volume > it->volume) {
                std::cout << "Volume greater than order volume: " << volume
                          << std::endl;
                return;
            }
            this->totalVolume -= (it->volume - volume);
            it->volume = volume;
            return;
        }
    }
}
// Postcondition: order.volume == volume

LimitOrderBook::LimitOrderBook(Exchange& exchange, std::string symbol,
                               double max_price, double tick)
    : exchange(exchange),
      bids(),
      asks(),
      orderIDs(),
      symbol(symbol),
      tick(tick),
      bestBid(),
      bestAsk(max_price),
      maxPrice(max_price) {
    size_t num_limits = static_cast<size_t>(max_price / tick) + 1;
    for (size_t i = 0; i <= num_limits; ++i) {
        bids.push_back(Limit(i * tick));
        asks.push_back(Limit(i * tick));
    }
}

// Might not be a good idea to expose Order datastruct to users and instead
// mebe should just have them provide params
void LimitOrderBook::addOrder(Order order) {
    if (order.price < 0) {
        std::cout << "Price below 0: " << order.price << std::endl;
        return;
    } else if (!divisible(order.price, this->tick)) {
        std::cout << "Price not rounded to tick: " << order.price << std::endl;
        return;
    }

    // Need to handle the price > maxprice case at some point
    size_t index = static_cast<size_t>(order.price / this->tick);

    bool bidOrderCrossedBook =
        (order.direction == Dir::BUY && order.price >= this->bestAsk);
    bool askOrderCrossedBook =
        (order.direction == Dir::SELL && order.price <= this->bestBid);
    if (bidOrderCrossedBook) {
        matchBidOrder(&order);
    } else if (askOrderCrossedBook) {
        matchAskOrder(&order);
    }

    if (order.volume > 0) {
        if (order.direction == Dir::BUY) {
            bids[index].addOrder(order);
            orderIDs[order.oid] = &bids[index];
            if (order.price > this->bestBid) {
                this->bestBid = order.price;
            }
        } else {
            asks[index].addOrder(order);
            orderIDs[order.oid] = &asks[index];
            if (order.price < this->bestAsk) {
                this->bestAsk = order.price;
            }
        }
        ADD add = {order.direction, order.price, order.volume, order.oid,
                   order.timestamp};
        this->addLog(add);
    }
}

void LimitOrderBook::cancelOrder(int oid) {
    if (orderIDs.find(oid) == orderIDs.end()) {
        std::cout << "Order ID not found: " << oid << std::endl;
        return;
    }
    Order order = this->getOrder(oid);
    CANCEL cancel = {order.direction, order.price, order.volume, oid,
                     order.timestamp};
    this->addLog(cancel);
    this->deleteOrder(oid);
}

void LimitOrderBook::editOrder(int oid, int volume) {
    if (orderIDs.find(oid) == orderIDs.end()) {
        std::cout << "Order ID not found: " << oid << std::endl;
        return;
    }
    Order order = this->getOrder(oid);
    if (volume > order.volume) {
        std::cout << "New volume greater than order volume: " << volume
                  << std::endl;
        return;
    }
    EDIT edit = {order.direction, order.price, order.volume,
                 volume,          oid,         order.timestamp};
    this->addLog(edit);
    Limit* limit = orderIDs[oid];
    limit->updateOrder(oid, volume);
}

std::string LimitOrderBook::getSymbol() { return this->symbol; }

// Wrap this in an optional!!!
Order LimitOrderBook::getOrder(int oid) {
    if (orderIDs.find(oid) == orderIDs.end()) {
        std::cout << "Order ID not found: " << oid << std::endl;
        return Order();
    }
    Limit* limit = orderIDs[oid];
    return limit->getOrder(oid);
}

double LimitOrderBook::getBestBid() { return this->bestBid; }

double LimitOrderBook::getBestAsk() { return this->bestAsk; }

void LimitOrderBook::printOrderBook() {
    std::cout << "Order Book:\n";
    std::cout << "------------------------------------\n";
    std::cout << std::left << std::setw(28) << "BIDS"
              << "    ASKS\n";
    std::cout << std::left << std::setw(5) << "Qty"
              << " | " << std::setw(8) << "Price"
              << " ||    Price |   Qty\n";
    std::cout << "------------------------------------\n";

    for (auto limit = asks.rbegin(); limit != asks.rend(); ++limit) {
        if (limit->getTotalVolume() > 0) {
            std::cout << std::left << std::setw(5) << " "
                      << " | " << std::setw(8) << " "
                      << " || " << std::right << std::setw(8) << std::fixed
                      << std::setprecision(2) << limit->getPrice() << " | "
                      << std::right << std::setw(5) << limit->getTotalVolume()
                      << '\n';
        }
    }

    for (auto limit = bids.rbegin(); limit != bids.rend(); ++limit) {
        if (limit->getTotalVolume() > 0) {
            std::cout << std::left << std::setw(5) << limit->getTotalVolume()
                      << " | " << std::setw(8) << std::fixed
                      << std::setprecision(2) << limit->getPrice() << " || "
                      << std::right << std::setw(8) << " "
                      << " | " << std::right << std::setw(5) << " " << '\n';
        }
    }
    std::cout << "------------------------------------\n";
}

// Precondition: Bid and ask are valid and bid->price >= ask->price
void LimitOrderBook::processTrade(Order* bid, Order* ask, Dir aggressor) {
    int volume = std::min(bid->volume, ask->volume);
    bid->volume -= volume;
    ask->volume -= volume;

    std::time_t timestamp;
    double trade_price;
    if (aggressor == Dir::BUY) {
        timestamp = bid->timestamp;
        trade_price = ask->price;
    } else {
        timestamp = ask->timestamp;
        trade_price = bid->price;
    }

    TRADE trade = {trade_price, volume,   timestamp,
                   aggressor,   bid->cid, ask->cid};
    this->addLog(trade);

    if (aggressor == Dir::SELL && bid->volume == 0) {
        this->deleteOrder(bid->oid);
    }
    if (aggressor == Dir::BUY && ask->volume == 0) {
        this->deleteOrder(ask->oid);
    }
}
// Postcondition: At least one of bid and ask has volume > 0 and is deleted

// Precondition: order is valid and order->price <=
// this->bestBid
void LimitOrderBook::matchBidOrder(Order* order) {
    while (order->volume > 0 && this->bestAsk <= order->price) {
        Limit* limit = &asks[static_cast<size_t>(this->bestAsk / this->tick)];
        Order priorityOrder = limit->getPriorityOrder();

        this->processTrade(order, &priorityOrder, Dir::BUY);

        // Is there a more efficient way than just iterating to the max price?
        // Maybe keeping track of some sort of bid and ask volume?
        // I'm also worried because this breaks the invariant that the bestAsk
        // is the bestAsk at all times. Maybe I can make This a critical
        // section?
        while (limit->getTotalVolume() == 0 &&
               this->bestAsk <= this->maxPrice) {
            this->bestAsk += this->tick;
            limit = &asks[static_cast<size_t>(this->bestAsk / this->tick)];
        }
    }
}
// Postcondition 1: order->price > bestBid

// Precondition: order is valid and order->price >= bestAsk
void LimitOrderBook::matchAskOrder(Order* order) {
    while (order->volume > 0 && this->bestBid >= order->price) {
        Limit* limit = &bids[static_cast<size_t>(this->bestBid / this->tick)];
        Order priorityOrder = limit->getPriorityOrder();

        this->processTrade(order, &priorityOrder, Dir::SELL);

        while (limit->getTotalVolume() == 0 && this->bestBid > 0) {
            this->bestBid -= this->tick;
            limit = &bids[static_cast<size_t>(this->bestBid / this->tick)];
        }
    }
}
// Postcondition 1: order->price < bestAsk

// Precondition: orderIDs[oid] exists
void LimitOrderBook::deleteOrder(int oid) {
    Limit* limit = orderIDs[oid];
    limit->removeOrder(oid);
    this->orderIDs.erase(oid);
}
// Postcondition 1: orderIDs[oid] does not exist and order has been removed from
// limit

void LimitOrderBook::addLog(TradeLog log) { this->exchange.broadcast(log); }

// What operations need to be broadcasted?
// 1. New order
// 2. Cancel order
// 3. Trade
// 4. Order Update
// 5. Aggressor Order

// I think the best way to deal with this is to send all these updates to a big
// queue and then have another class deal with it.