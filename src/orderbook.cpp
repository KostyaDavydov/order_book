#include "orderbook.h"

#include <cmath>

//=================================================
//=================================================

void OrderBook::add_order(OrderID id, double price, std::size_t volume, OrderType type)
{

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

}

//=================================================

double OrderBook::best_ask() const
{

}

//=================================================

std::size_t OrderBook::volume_at_price(double price) const
{

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
