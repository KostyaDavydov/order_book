#pragma once

#include <map>
#include <unordered_map>
#include <list>

//=================================================
//=================================================

using OrderID = std::size_t;
enum class OrderType {ASK, BID};

//=================================================
//=================================================

class OrderBook
{
private:
// Additional data structures
    struct Order
    {
        double m_price; // the price corresponding to the given order
        std::size_t m_volume; // volume of a product for the given order
        OrderType m_type; // ASK/BID (see OrderType enum)
    };

    struct PriceLevel
    {
        std::size_t m_volume; // total volume at the given price
        std::list<Order> m_orders; // list of all the orders at the given level
    };

// Additional alias
    using OrderIter = std::list<Order>::iterator;

// Fields
    std::map<double, PriceLevel, std::greater<double>> m_bids;
    std::map<double, PriceLevel, std::less<double>> m_asks;
    std::unordered_map<OrderID, OrderIter> m_orderIterators;


public:
    OrderBook() = default;
    OrderBook(const OrderBook &) = delete;
    OrderBook & operator=(const OrderBook &) = delete;
    OrderBook(OrderBook &&) = default;
    OrderBook & operator=(OrderBook &&) = default;
    ~OrderBook() = default;

    // Add an order with the given parameters to the order book
    void add_order(OrderID id, double price, std::size_t volume, OrderType type);

    // Delete the order with the given id from the order book
    void delete_order(OrderID id);

    // Change the order with the given id (new values for price and volume could be set)
    void modify_order(OrderID id, double newPrice, std::size_t newVolume);

    // Execute (maybe partially: then the execVolume is given) the order with the given id
    void execute_order(OrderID id, std::size_t execVolume = 0);

    // Get the best (lowest) bid price
    double best_bid() const;

    // Get the best (highest) ask price
    double best_ask() const;

    // Get the product volume at the given price
    std::size_t volume_at_price(double price) const;
};
