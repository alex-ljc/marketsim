#include <cmath>
#include <deque>
#include <iomanip>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

enum class Dir { BUY, SELL };

struct Order {
    Dir direction;
    double price;
    int quantity;
    int timestamp;
    int oid;
    int cid;
};

bool isEqual(double a, double b) {
    if (std::fpclassify(a) != std::fpclassify(b)) {
        // Different classifications, not equal
        return false;
    }

    return std::fabs(a - b) < std::numeric_limits<double>::epsilon();
}

namespace {
class Limit {
   public:
    Limit(double price) : orders(), price(price), totalVolume(0) {}
    double getPrice() { return this->price; }

    int getTotalVolume() { return this->totalVolume; }

    void addOrder(Order order) {
        if (!isEqual(order.price, this->price)) {
            std::cout << "Order has wrong price: " << order.price << std::endl;
            return;
        }

        this->price = order.price;
        this->totalVolume += order.quantity;
        this->orders.push_back(order);
    }

    void cancelOrder(int oid) {
        for (auto it = this->orders.begin(); it != this->orders.end(); ++it) {
            if (it->oid == oid) {
                this->orders.erase(it);
                return;
            }
        }
    }

    Order getOrder(int oid) {
        for (auto it = this->orders.begin(); it != this->orders.end(); ++it) {
            if (it->oid == oid) {
                return *it;
            }
        }
        return Order();
    }

   private:
    std::deque<Order> orders;
    double price;
    int totalVolume;
};
}  // namespace

class LimitOrderBook {
   public:
    LimitOrderBook(double max_price, double tick)
        : bids(static_cast<size_t>(max_price / tick)),
          asks(static_cast<size_t>(max_price / tick)),
          orderIDs(),
          tick(tick),
          bestBid(0),
          bestAsk(max_price) {}

    // Might not be a good idea to expose Order datastruct to users and instead
    // mebe should just have them provide params
    void addOrder(Order order) {
        if (order.price < 0) {
            std::cout << "Invalid price: " << order.price << std::endl;
            return;
        } else if (std::fmod(order.price, this->tick) != 0) {
            std::cout << "Invalid price: " << order.price << std::endl;
            return;
        }
        // Need to handle the price > maxprice case at some point
        size_t index = static_cast<size_t>(order.price / this->tick);
        if (order.direction == Dir::BUY) {
            bids[index].addOrder(order);
        } else {
            asks[index].addOrder(order);
        }
    }

    void cancelOrder(int oid) {
        if (orderIDs.find(oid) == orderIDs.end()) {
            std::cout << "Order ID not found: " << oid << std::endl;
            return;
        }
        Limit* limit = orderIDs[oid];
        limit->cancelOrder(oid);
    }

    Order getOrder(int oid) {
        if (orderIDs.find(oid) == orderIDs.end()) {
            std::cout << "Order ID not found: " << oid << std::endl;
            return Order();
        }
        Limit* limit = orderIDs[oid];
        return limit->getOrder(oid);
    }

    double getBestBid() { return this->bestBid; }

    double getBestAsk() { return this->bestAsk; }

    void printOrderBook() {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Order Book:\n";
        std::cout << "-------------------------------------\n";
        std::cout << std::left << std::setw(28) << "BIDS"
                  << "ASKS\n";
        std::cout << std::left << std::setw(8) << "Qty"
                  << "| " << std::setw(8) << "Price"
                  << "|| Price     | Qty\n";
        std::cout << "-------------------------------------\n";

        for (auto limit : asks) {
            if (limit.getTotalVolume() > 0) {
                std::cout << std::left << std::setw(8) << "\t\t| "
                          << std::setw(8) << "|| " << std::right << std::setw(8)
                          << limit.getTotalVolume() << " | " << std::right
                          << std::setw(8) << limit.getPrice() << '\n';
            }
        }
        for (auto limit : bids) {
            if (limit.getTotalVolume() > 0) {
                std::cout << std::left << std::setw(8) << limit.getPrice()
                          << " | " << std::setw(8) << limit.getTotalVolume()
                          << " ||\t\t\t\n";
            }
        }
        std::cout << "-------------------------------------\n";
    }

   private:
    std::vector<Limit> bids;
    std::vector<Limit> asks;
    std::unordered_map<int, Limit*> orderIDs;
    double tick;
    // Should these be limits or prices?
    double bestBid;
    double bestAsk;
};

int main() {
    LimitOrderBook lob(100, 0.01);
    Order order;
    order.direction = Dir::BUY;
    order.price = 10.0;
    order.quantity = 100;
    order.timestamp = 0;
    order.oid = 0;
    order.cid = 0;
    lob.addOrder(order);
    lob.printOrderBook();
    return 0;
}
