#include "orderbook.h"

#include <cmath>

//=================================================
//=================================================

void OrderBook::add_order(OrderID id, double price, std::size_t volume, OrderType type)
{
    // Validate input
    if (price <= 0.0 || volume == 0)
        return;

    // Convert the price to the integer representation
    Price intPrice = double_to_price(price);

    // Create a new order object
    Order newOrder {
        intPrice,
        volume,
        type
    };

    // Define the type (side) of the order
    switch(type)
    {
    case OrderType::ASK:
    {
        auto priceLvlIter = m_asks.find(intPrice); // search the order with the given price
        if (priceLvlIter == m_asks.end()) // if there is no order with that price in the map
        {
            // Create a new PriceLevel object
            m_asks[intPrice] = PriceLevel { volume,
                                            {std::move(newOrder)}
                                          };
            m_orderIterators[id] = m_asks[intPrice].m_orders.begin(); // create a reference (iterator) to the order
        }
        else // otherwise there are added orders with that price in the map
        {
            priceLvlIter->second.m_volume += volume; // update the total volume
            priceLvlIter->second.m_orders.push_front(std::move(newOrder)); // insert the new order at the beginning
            m_orderIterators[id] = priceLvlIter->second.m_orders.begin(); // create a reference (iterator) to the order
        }
        return;
    }
    case OrderType::BID:
    {
        auto priceLvlIter = m_bids.find(intPrice); // search the order with the given price
        if (priceLvlIter == m_bids.end()) // if there is no order with that price in the map
        {
            // Create a new PriceLevel object
            m_bids[intPrice] = PriceLevel { volume,
                                            {std::move(newOrder)}
                                          };
            m_orderIterators[id] = m_bids[intPrice].m_orders.begin(); // create a reference (iterator) to the order
        }
        else // otherwise there are added orders with that price in the map
        {
            priceLvlIter->second.m_volume += volume; // update the total volume
            priceLvlIter->second.m_orders.push_front(std::move(newOrder)); // insert the new order at the beginning
            m_orderIterators[id] = priceLvlIter->second.m_orders.begin(); // create a reference (iterator) to the order
        }
        return;
    }
    }
}

//=================================================

void OrderBook::delete_order(OrderID id)
{
    // Search the order with the given id (get the unordered map iterator)
    auto orderToDelete_MapIter = m_orderIterators.find(id);

    if (orderToDelete_MapIter != m_orderIterators.end()) // if the order was found
    {
        // Get the iterator to the list element
        auto orderToDelete_ListIter = orderToDelete_MapIter->second;

        // Define the type (side) of the order and remove the element from the corresponding map
        switch(orderToDelete_ListIter->m_type)
        {
        case OrderType::ASK:
        {
            Price currPrice = orderToDelete_ListIter->m_price;
            auto & currPriceLevel = m_asks[currPrice];

            currPriceLevel.m_volume -= orderToDelete_ListIter->m_volume;
            currPriceLevel.m_orders.erase(orderToDelete_ListIter);

            if (currPriceLevel.m_orders.empty())
                m_asks.erase(currPrice);
            break;
        }
        case OrderType::BID:
        {
            Price currPrice = orderToDelete_ListIter->m_price;
            auto & currPriceLevel = m_bids[currPrice];

            currPriceLevel.m_volume -= orderToDelete_ListIter->m_volume;
            currPriceLevel.m_orders.erase(orderToDelete_ListIter);

            if (currPriceLevel.m_orders.empty())
                m_bids.erase(currPrice);
            break;
        }
        }
        m_orderIterators.extract(id);
    }
}

//=================================================

void OrderBook::modify_order(OrderID id, double newPrice, std::size_t newVolume)
{
    // Validate input
    if (newPrice <= 0.0 || newVolume == 0)
        return;

    // Search the order with the given id (get the unordered map iterator)
    auto orderToModify_MapIter = m_orderIterators.find(id);

    if (orderToModify_MapIter != m_orderIterators.end()) // if the order was found
    {
        // Get the corresponding order
        Order orderToModify = *(orderToModify_MapIter->second);

        if (orderToModify.m_price != double_to_price(newPrice)) // if the price was modified
        {
            delete_order(id);
            add_order(id, newPrice, newVolume, orderToModify.m_type);
        }
        else // otherwise it's enough to change only the total volume for the price level
        {
            // Convertion to `long` is used to handle negative values
            switch (orderToModify.m_type)
            {
            case OrderType::ASK:
                orderToModify_MapIter->second->m_volume = newVolume;
                m_asks[orderToModify.m_price].m_volume += (long(newVolume) - orderToModify.m_volume);
                break;
            case OrderType::BID:
                orderToModify_MapIter->second->m_volume = newVolume;
                m_bids[orderToModify.m_price].m_volume += (long(newVolume) - orderToModify.m_volume);
                break;
            }
        }
    }
}

//=================================================

void OrderBook::execute_order(OrderID id, std::size_t execVolume)
{
    // Search the order with the given id (get the unordered map iterator)
    auto orderToModify_MapIter = m_orderIterators.find(id);

    if (orderToModify_MapIter != m_orderIterators.end()) // if the order was found
    {
        // Get the corresponding order
        Order orderToExec = *(orderToModify_MapIter->second);

        if (execVolume == orderToExec.m_volume || execVolume == 0)
            delete_order(id); // delete the order from the book
        else if (execVolume < orderToExec.m_volume)
            modify_order(id, price_to_double(orderToExec.m_price), orderToExec.m_volume - execVolume); // just change the volume
        else
            return; // not a valid value
    }
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
    if (val < 0)
        return static_cast<Price>(0);
    else
        return static_cast<Price>(std::round(val * 1e4));
}

//=================================================

double OrderBook::price_to_double(Price val) const noexcept
{
    return static_cast<double>(val) / 1e4;
}

//=================================================
