#include "orderbook.h"

#include <cmath>

//=================================================
//=================================================

void OrderBook::add_order(OrderID id, double price, std::size_t volume, OrderType type)
{
    // Validate input
    if (price <= 0.0 || volume == 0)
        return;

    // Create a new order object
    Order newOrder {
        double_to_price(price),
        volume,
        type
    };

    // Define the type (side) of the order
    switch(type)
    {
    case OrderType::ASK:
    {
        auto priceLvlIter = m_asks.find(newOrder.m_price); // search the order with the given price
        if (priceLvlIter == m_asks.end()) // if there is no order with that price in the map
        {
            // Create a new PriceLevel object
            m_asks[newOrder.m_price] = PriceLevel { newOrder.m_volume,
                                                    {std::move(newOrder)}
                                                  };
        }
        else // otherwise there are added orders with that price in the map
        {
            priceLvlIter->second.m_volume += newOrder.m_volume; // update the total volume
            priceLvlIter->second.m_orders.push_front(std::move(newOrder)); // insert the new order at the beginning
        }
        m_orderIterators[id] = priceLvlIter->second.m_orders.begin(); // create a reference (iterator) to the order
        break;
    }
    case OrderType::BID:
    {
        auto priceLvlIter = m_bids.find(newOrder.m_price); // search the order with the given price
        if (priceLvlIter == m_bids.end()) // if there is no order with that price in the map
        {
            // Create a new PriceLevel object
            m_bids[newOrder.m_price] = PriceLevel { newOrder.m_volume,
                                                    {std::move(newOrder)}
                                                  };
        }
        else // otherwise there are added orders with that price in the map
        {
            priceLvlIter->second.m_volume += newOrder.m_volume; // update the total volume
            priceLvlIter->second.m_orders.push_front(std::move(newOrder)); // insert the new order at the beginning
        }
        m_orderIterators[id] = priceLvlIter->second.m_orders.begin(); // create a reference (iterator) to the order
        break;
    }
    }
}

//=================================================

void OrderBook::delete_order(OrderID id)
{

}

//=================================================

void OrderBook::modify_order(OrderID id, double newPrice, std::size_t newVolume)
{

}

//=================================================

void OrderBook::execute_order(OrderID id, std::size_t execVolume)
{

}

//=================================================

double OrderBook::best_bid() const
{
    // Return the first key in the map if non-empty
    if (m_bids.empty())
        return 0.0;
    else
        return price_to_double(m_bids.begin()->first);
}

//=================================================

double OrderBook::best_ask() const
{
    // Return the first key in the map if non-empty
    if (m_asks.empty())
        return 0.0;
    else
        return price_to_double(m_asks.begin()->first);
}

//=================================================

std::size_t OrderBook::volume_at_price(double price) const
{
    // Queries total volume across both sides for a given price

    auto priceLvlIter_ask = m_asks.find(double_to_price(price));
    if (priceLvlIter_ask != m_asks.end()) // if there is an order with the given price in the asks map
        return priceLvlIter_ask->second.m_volume;

    auto priceLvlIter_bid = m_bids.find(double_to_price(price));
    if (priceLvlIter_bid != m_bids.end()) // if there is an order with the given price in the bids map
        return priceLvlIter_bid->second.m_volume;

    return 0; // no orders with the given price
}

//=================================================

Price OrderBook::double_to_price(double val) const noexcept
{
    return static_cast<Price>(std::round(val * 1e4));
}

//=================================================

double OrderBook::price_to_double(Price val) const noexcept
{
    return static_cast<double>(val) / 1e4;
}

//=================================================
