#include "gtest/gtest.h"
#include "Bot.h"


class TestSim: public IDvfSimulator{
public:
    virtual ~TestSim() noexcept = default;

    static IDvfSimulator* Create() noexcept
    {
        return static_cast<IDvfSimulator*>(new TestSim());
    }

    virtual OrderBook GetOrderBook() noexcept override final
    {
        return m_ob;
    }
    virtual std::optional<OrderID> PlaceOrder(double price, double amount) noexcept override final
    {
        m_ob.push_back(std::make_pair(price, amount));
        return true;
    }
    virtual bool CancelOrder(OrderID oid) noexcept override final
    {
        return false;
    }

private:
    TestSim() noexcept
    {

        for (double i = -10; i < 11; ++i)
        {

            if (i == 0)
                ++i;

            m_ob.push_back(std::make_pair(100. - i, i));

        }
        
    }
    OrderBook m_ob;

};
TEST(TestBot, FindBestBidAsk)
{
    auto sim = TestSim::Create();
    SimpleBot bot(sim);
    bot.Update();
    EXPECT_EQ(bot.GetBestBid().first, 99);
    EXPECT_EQ(bot.GetBestAsk().first, 101);
}

TEST(TestBot, FindBestBidAskAfterOrder)
{
    auto sim = TestSim::Create();
    SimpleBot bot(sim);
    bot.Update();
    sim->PlaceOrder(102, -1);
    sim->PlaceOrder(98, 1);

    EXPECT_EQ(bot.GetBestBid().first, 99);
    EXPECT_EQ(bot.GetBestAsk().first, 101);
}