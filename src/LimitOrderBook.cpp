#include <chrono>
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
    int volume;
    std::time_t timestamp;
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

class Limit {
   public:
    Limit() : orders(), price(0), totalVolume(0) {}

    Limit(double price) : orders(), price(price), totalVolume(0) {}
    double getPrice() { return this->price; }

    int getTotalVolume() { return this->totalVolume; }

    void addOrder(Order order) {
        if (!isEqual(order.price, this->price)) {
            std::cout << "Order has wrong price: " << order.price << std::endl;
            std::cout << "Limit price: " << this->price << std::endl;
            return;
        }

        this->price = order.price;
        this->totalVolume += order.volume;
        this->orders.push_back(order);
    }

    void removeOrder(int oid) {
        for (auto it = this->orders.begin(); it != this->orders.end(); ++it) {
            if (it->oid == oid) {
                this->orders.erase(it);
                this->totalVolume -= it->volume;
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

    Order getPriorityOrder() { return this->orders.front(); }

    void updateOrder(int oid, int volume) {
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

   private:
    std::deque<Order> orders;
    double price;
    int totalVolume;
};

class LimitOrderBook {
   public:
    LimitOrderBook(double max_price, double tick)
        : bids(),
          asks(),
          orderIDs(),
          nextOID(0),
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
    void addOrder(Dir direction, double price, int volume, int cid) {
        if (price < 0) {
            std::cout << "Price below 0: " << price << std::endl;
            return;
        } else if (!divisible(price, this->tick)) {
            std::cout << "Price not rounded to tick: " << price << std::endl;
            return;
        }

        // Need to handle the price > maxprice case at some point
        size_t index = static_cast<size_t>(price / this->tick);

        std::time_t timestamp = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
        int oid = nextOID++;
        Order order = {direction, price, volume, timestamp, oid, cid};

        bool bidOrderCrossedBook =
            (direction == Dir::BUY && price >= this->bestAsk);
        bool askOrderCrossedBook =
            (direction == Dir::SELL && price <= this->bestBid);
        if (bidOrderCrossedBook) {
            matchBidOrder(&order);
        } else if (askOrderCrossedBook) {
            matchAskOrder(&order);
        }

        if (order.volume > 0) {
            if (direction == Dir::BUY) {
                bids[index].addOrder(order);
                orderIDs[order.oid] = &bids[index];
                if (price > this->bestBid) {
                    this->bestBid = price;
                }
            } else {
                asks[index].addOrder(order);
                orderIDs[order.oid] = &asks[index];
                if (price < this->bestAsk) {
                    this->bestAsk = price;
                }
            }
        }
    }

    void cancelOrder(int oid) {
        if (orderIDs.find(oid) == orderIDs.end()) {
            std::cout << "Order ID not found: " << oid << std::endl;
            return;
        }
        this->deleteOrder(oid);
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
        std::cout << "Order Book:\n";
        std::cout << "------------------------------------\n";
        std::cout << std::left << std::setw(28) << "BIDS"
                  << "    ASKS\n";
        std::cout << std::left << std::setw(5) << "Qty"
                  << " | " << std::setw(8) << "Price"
                  << " ||    Price |   Qty\n";
        std::cout << "------------------------------------\n";

        for (auto limit : asks) {
            if (limit.getTotalVolume() > 0) {
                std::cout << std::left << std::setw(5) << " "
                          << " | " << std::setw(8) << " "
                          << " || " << std::right << std::setw(8) << std::fixed
                          << std::setprecision(2) << limit.getPrice() << " | "
                          << std::right << std::setw(5)
                          << limit.getTotalVolume() << '\n';
            }
        }

        for (auto limit = bids.rbegin(); limit != bids.rend(); ++limit) {
            if (limit->getTotalVolume() > 0) {
                std::cout << std::left << std::setw(5)
                          << limit->getTotalVolume() << " | " << std::setw(8)
                          << std::fixed << std::setprecision(2)
                          << limit->getPrice() << " || " << std::right
                          << std::setw(8) << " "
                          << " | " << std::right << std::setw(5) << " " << '\n';
            }
        }
        std::cout << "------------------------------------\n";
    }

   private:
    std::vector<Limit> bids;
    std::vector<Limit> asks;
    std::unordered_map<int, Limit*> orderIDs;
    int nextOID;
    double tick;
    // Should these be limits or prices?
    double bestBid;
    double bestAsk;
    double maxPrice;

    void matchBidOrder(Order* order) {
        while (order->volume > 0 && this->bestAsk <= order->price) {
            Limit* limit =
                &asks[static_cast<size_t>(this->bestAsk / this->tick)];
            Order priorityOrder = limit->getPriorityOrder();

            bool priorityOrderConsumed = priorityOrder.volume <= order->volume;
            if (priorityOrderConsumed) {
                order->volume -= priorityOrder.volume;
                this->deleteOrder(priorityOrder.oid);

                double newBestAsk = limit->getPrice();
                while (limit->getTotalVolume() == 0 &&
                       newBestAsk <= this->maxPrice) {
                    newBestAsk += this->tick;
                    limit = &asks[static_cast<size_t>(newBestAsk / this->tick)];
                }
                this->bestAsk = newBestAsk;
            } else {
                limit->updateOrder(priorityOrder.oid,
                                   priorityOrder.volume - order->volume);
                order->volume = 0;
            }
        }
    }

    void matchAskOrder(Order* order) {
        while (order->volume > 0 && this->bestBid >= order->price) {
            Limit* limit =
                &bids[static_cast<size_t>(this->bestBid / this->tick)];
            Order priorityOrder = limit->getPriorityOrder();

            bool priorityOrderConsumed = priorityOrder.volume <= order->volume;
            if (priorityOrderConsumed) {
                order->volume -= priorityOrder.volume;
                this->deleteOrder(priorityOrder.oid);

                double newBestBid = limit->getPrice();
                while (limit->getTotalVolume() == 0 && newBestBid >= 0) {
                    newBestBid -= this->tick;
                    limit = &bids[static_cast<size_t>(newBestBid / this->tick)];
                }
                this->bestBid = newBestBid;
            } else {
                limit->updateOrder(priorityOrder.oid,
                                   priorityOrder.volume - order->volume);
                order->volume = 0;
            }
        }
    }

    void deleteOrder(int oid) {
        if (orderIDs.find(oid) == orderIDs.end()) {
            std::cout << "Order ID not found: " << oid << std::endl;
            return;
        }
        Limit* limit = orderIDs[oid];
        limit->removeOrder(oid);
        this->orderIDs.erase(oid);
    }
};
