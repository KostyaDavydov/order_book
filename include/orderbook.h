#pragma once

#include <map>
#include <unordered_map>
#include <list>
#include <cstdint>
#include <vector>

//=================================================
//=================================================

using Price = std::uint64_t;
using OrderID = std::size_t;
using PriceLevelsVector = std::vector<std::pair<double, std::size_t>>;
enum class OrderType {ASK, BID};

//=================================================
//=================================================

class OrderBook
{
private:
// Additional data structures
    struct Order
    {
        OrderID m_id; // id of the order
        Price m_price; // the price corresponding to the given order
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
    std::map<Price, PriceLevel, std::greater<Price>> m_bids;
    std::map<Price, PriceLevel, std::less<Price>> m_asks;
    std::unordered_map<OrderID, OrderIter> m_orderIterators;
    double m_marketPrice {-1.0}; // the default value means that there weren't
                                 // any transactions yet

// Methods
    Price double_to_price(double val) const noexcept;
    double price_to_double(Price val) const noexcept;

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

    // Try to execute the given volume from the given
    // side (type) and return the actually executed volume
    std::size_t execute(OrderType type, std::size_t execVolume);

    // Get the best (lowest) bid price
    double best_bid() const;

    // Get the best (highest) ask price
    double best_ask() const;

    // Get the current spread
    double spread() const noexcept;

    // Get the current market price
    double market_price() const noexcept;

    // Get the product volume at the given price
    std::size_t volume_at_price(double price) const;

    // Get all price levels for the given orders type
    PriceLevelsVector price_levels_for_type(OrderType type) const;
};
