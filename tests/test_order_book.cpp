#include <gtest/gtest.h>

#include "orderbook.h"

//=================================================
//=================================================

class OrderBookTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        book = new OrderBook();
    }

    void TearDown() override
    {
        delete book;
    }

    // Helper method to add multiple ASK orders
    void addAskOrders() {
        book->add_order(1, 100.0, 7, OrderType::ASK);
        book->add_order(2, 101.5, 5, OrderType::ASK);
        book->add_order(3, 100.0, 8, OrderType::ASK);  // Same price as order 1
    }

    // Helper method to add multiple BID orders
    void addBidOrders() {
        book->add_order(4, 99.5, 15, OrderType::BID);
        book->add_order(5, 98.0, 20, OrderType::BID);
        book->add_order(6, 99.5, 10, OrderType::BID);  // Same price as order 4
    }

    // Helper method to add mixed orders
    void addMixedOrders() {
        book->add_order(1, 100.0, 10, OrderType::ASK);
        book->add_order(2, 99.5 , 15, OrderType::BID);
        book->add_order(3, 101.0,  5, OrderType::ASK);
        book->add_order(4, 99.0 , 20, OrderType::BID);
        book->add_order(5, 101.0, 10, OrderType::ASK);
        book->add_order(6, 99.0 , 20, OrderType::BID);
    }

    OrderBook * book;
};

//=================================================
//=================================================

const double PRICE_ACCURACY = 1e-5;

//=================================================
// Test 1: Adding single ASK order
//=================================================
TEST_F(OrderBookTest, AddSingleAskOrder)
{
    // Arrrange
    OrderID id = 1;
    double price = 100.0;
    std::size_t volume = 10;

    // Act
    book->add_order(id, price, volume, OrderType::ASK);

    // Assert
    EXPECT_NEAR(book->best_ask(), price, PRICE_ACCURACY);
    EXPECT_NEAR(book->best_bid(), 0.0, PRICE_ACCURACY);
    EXPECT_EQ(book->volume_at_price(price), volume);
}

//=================================================
// Test 2: Adding single BID order
//=================================================
TEST_F(OrderBookTest, AddSingleBidOrder)
{
    // Arrrange
    OrderID id = 1;
    double price = 100.0;
    std::size_t volume = 10;

    // Act
    book->add_order(id, price, volume, OrderType::BID);

    // Assert
    EXPECT_NEAR(book->best_bid(), price, PRICE_ACCURACY);
    EXPECT_NEAR(book->best_ask(), 0.0, PRICE_ACCURACY);
    EXPECT_EQ(book->volume_at_price(price), volume);
}

//=================================================
// Test 3: Adding several ASK orders
//=================================================
TEST_F(OrderBookTest, AddSeveralAskOrders)
{
    // Act
    addAskOrders();

    // Assert
    EXPECT_NEAR(book->best_ask(), 100.0, PRICE_ACCURACY);
    EXPECT_NEAR(book->best_bid(), 0.0, PRICE_ACCURACY);
    EXPECT_EQ(book->volume_at_price(100.0), 15);
}

//=================================================
// Test 4: Adding several BID orders
//=================================================
TEST_F(OrderBookTest, AddSeveralBidOrders)
{
    // Act
    addBidOrders();

    // Assert
    EXPECT_NEAR(book->best_ask(), 0.0, PRICE_ACCURACY);
    EXPECT_NEAR(book->best_bid(), 99.5, PRICE_ACCURACY);
    EXPECT_EQ(book->volume_at_price(99.5), 25);
}

//=================================================
// Test 5: Adding several orders (both types)
//=================================================
TEST_F(OrderBookTest, AddSeveralMixedOrders)
{
    // Act
    addMixedOrders();

    // Assert
    EXPECT_NEAR(book->best_ask(), 100.0, PRICE_ACCURACY);
    EXPECT_NEAR(book->best_bid(), 99.5, PRICE_ACCURACY);
    EXPECT_EQ(book->volume_at_price(99.0), 40);
    EXPECT_EQ(book->volume_at_price(101.0), 15);
}

//=================================================
// Test 6: Adding orders with invalid values
//=================================================
TEST_F(OrderBookTest, AddOrdersInvalidValues)
{
    // Act
    book->add_order(1, -5.0, 5, OrderType::ASK);
    book->add_order(2,  5.0, 0, OrderType::BID);

    // Assert
    EXPECT_NEAR(book->best_ask(), 0.0, PRICE_ACCURACY);
    EXPECT_NEAR(book->best_bid(), 0.0, PRICE_ACCURACY);
    EXPECT_EQ(book->volume_at_price(-5.0), 0.0);
    EXPECT_EQ(book->volume_at_price(5.0), 0.0);
}

//=================================================
// Test 7: Adding some orders and deleting one of them
//=================================================
TEST_F(OrderBookTest, AddManyDeleteOne)
{
    // Act
    addAskOrders();
    book->delete_order(3);

    // Assert
    EXPECT_NEAR(book->best_ask(), 100.0, PRICE_ACCURACY);
    EXPECT_NEAR(book->best_bid(), 0.0, PRICE_ACCURACY);
    EXPECT_EQ(book->volume_at_price(100.0), 7);
}

//=================================================
// Test 8: Adding some orders and deleting some of them
//=================================================
TEST_F(OrderBookTest, AddManyDeleteMany)
{
    // Act
    addMixedOrders();

    book->delete_order(1);
    book->delete_order(4);
    book->delete_order(6);

    // Assert
    EXPECT_NEAR(book->best_ask(), 101.0, PRICE_ACCURACY);
    EXPECT_NEAR(book->best_bid(), 99.5, PRICE_ACCURACY);
    EXPECT_EQ(book->volume_at_price(100.0), 0);
    EXPECT_EQ(book->volume_at_price(101.0), 15);
    EXPECT_EQ(book->volume_at_price(99.0), 0);
    EXPECT_EQ(book->volume_at_price(99.5), 15);
 }
