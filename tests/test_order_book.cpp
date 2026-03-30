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

    // Helper method to add many mixed orders
    void addManyMixedOrders() {
        // After the whole addition process we have 10 price levels (10.0, 20.0, ..., 100.0).
        // The orders with prices <= 50.0 are BIDS, others are ASKS.
        // On each price level we have 5 orders with volumes 1, 2, ..., 5 (so the total volume
        // on each level is 15).
        // -------------------------------
        // |VOLUME(BID)|PRICE|VOLUME(ASK)|
        // -------------------------------
        // |           |100.0|    15     |
        // |           | 90.0|    15     |
        // |           | 80.0|    15     |
        // |           | 70.0|    15     |
        // |           | 60.0|    15     |
        // |    15     | 50.0|           |
        // |    15     | 40.0|           |
        // |    15     | 30.0|           |
        // |    15     | 20.0|           |
        // |    15     | 10.0|           |
        // -------------------------------

        OrderID id = 1;
        for (double price = 10.0; price <= 100.0; price += 10.0)
        {
            for (int j = 1; j <= 5; ++j)
            {
                book->add_order(id, price, j, price <= 50 ? OrderType::BID : OrderType::ASK);
                ++id;
            }
        }
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

//=================================================
// Test 9: Adding some asks and modifying some of them (volume only)
//=================================================
TEST_F(OrderBookTest, AddAsksModifyVolumeOnly)
{
    // Act
    addAskOrders();

    book->modify_order(1, 100.0, 20);
    book->modify_order(2, 101.5, 2);

    // Assert
    EXPECT_NEAR(book->best_ask(), 100.0, PRICE_ACCURACY);
    EXPECT_EQ(book->volume_at_price(100.0), 28);
    EXPECT_EQ(book->volume_at_price(101.5), 2);
}

//=================================================
// Test 10: Adding some bids and modifying some of them (price and volume)
//=================================================
TEST_F(OrderBookTest, AddBidsModifyPriceAndVolume)
{
    // Act
    addBidOrders();

    book->modify_order(4, 100.0, 20);
    book->modify_order(5, 97.0, 10);

    // Assert
    EXPECT_NEAR(book->best_bid(), 100.0, PRICE_ACCURACY);
    EXPECT_EQ(book->volume_at_price(100.0), 20);
    EXPECT_EQ(book->volume_at_price(99.5), 10);
    EXPECT_EQ(book->volume_at_price(98.0), 0);
    EXPECT_EQ(book->volume_at_price(97.0), 10);
}

//=================================================
// Test 11: Adding many orders, deleting, and modifying some of them
//=================================================
TEST_F(OrderBookTest, AddManyDeleteModifySome)
{
    // Act
    addManyMixedOrders();

    // Modify the whole first price level (10.0 -> 55.0) so the
    // orders become on the highest BID level in the order book
    book->modify_order(1, 55.0, 1);
    book->modify_order(2, 55.0, 2);
    book->modify_order(3, 55.0, 3);
    book->modify_order(4, 55.0, 4);
    book->modify_order(5, 55.0, 5);

    // Delete part of the BIDS from the 50.0 price level
    book->delete_order(22); // volume -2
    book->delete_order(23); // volume -3
    book->delete_order(24); // volume -4

    // Delete all the ASKS from the 60.0 price level
    book->delete_order(26);
    book->delete_order(27);
    book->delete_order(28);
    book->delete_order(29);
    book->delete_order(30);

    // Modify part of the ASKS on the 70.0 price level
    book->modify_order(31, 80.0, 1);
    book->modify_order(32, 80.0, 2);

    // Assert
    EXPECT_NEAR(book->best_bid(), 55.0, PRICE_ACCURACY);
    EXPECT_NEAR(book->best_ask(), 70.0, PRICE_ACCURACY);
    EXPECT_EQ(book->volume_at_price(10.0), 0);
    EXPECT_EQ(book->volume_at_price(50.0), 6);
    EXPECT_EQ(book->volume_at_price(60.0), 0);
    EXPECT_EQ(book->volume_at_price(70.0), 12);
    EXPECT_EQ(book->volume_at_price(80.0), 18);
}

//=================================================
// Test 12: Adding several ASK orders, executing one of them
//=================================================
TEST_F(OrderBookTest, AddAsksExecOne)
{
    // Act
    addAskOrders();

    book->execute_order(2, 4);

    // Assert
    EXPECT_NEAR(book->best_ask(), 100.0, PRICE_ACCURACY);
    EXPECT_NEAR(book->best_bid(), 0.0, PRICE_ACCURACY);
    EXPECT_EQ(book->volume_at_price(100.0), 15);
    EXPECT_EQ(book->volume_at_price(101.5), 1);
}

//=================================================
// Test 13: Adding several ASK orders, try to execute invalid volume
//=================================================
TEST_F(OrderBookTest, AddAsksInvalidExec)
{
    // Act
    addAskOrders();

    book->execute_order(1, 8);

    EXPECT_EQ(book->volume_at_price(100.0), 15);
}

//=================================================
// Test 14: Adding some orders and executing some of them
//=================================================
TEST_F(OrderBookTest, AddMixedExecSome)
{
    // Act
    addMixedOrders();

    book->execute_order(1);
    book->execute_order(4, 20);

    // Assert
    EXPECT_NEAR(book->best_ask(), 101.0, PRICE_ACCURACY);
    EXPECT_NEAR(book->best_bid(), 99.5, PRICE_ACCURACY);
    EXPECT_EQ(book->volume_at_price(100.0), 0);
    EXPECT_EQ(book->volume_at_price(99.0), 20);
}

//=================================================
// Test 15: Adding many orders, executing some of them
//=================================================
TEST_F(OrderBookTest, AddManyExecuteSome)
{
    // Act
    addManyMixedOrders();

    // Execute some orders from the first price level (10.0)
    book->execute_order(3, 2); // 2 from 3
    book->execute_order(4); // 4 from 4

    // Execute all BIDS from their heighest level
    book->execute_order(21);
    book->execute_order(22);
    book->execute_order(23);
    book->execute_order(24);
    book->execute_order(25);

    // Execute some orders from 80.0 price level
    book->execute_order(40, 5);

    // Assert
    EXPECT_NEAR(book->best_ask(), 60.0, PRICE_ACCURACY);
    EXPECT_NEAR(book->best_bid(), 40.0, PRICE_ACCURACY);
    EXPECT_EQ(book->volume_at_price(10.0), 9);
    EXPECT_EQ(book->volume_at_price(50.0), 0);
    EXPECT_EQ(book->volume_at_price(80.0), 10);
}
