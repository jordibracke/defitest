#include "Bot.h"
#include <algorithm>
#include <assert.h>

SimpleBot::SimpleBot(IDvfSimulator* sim) noexcept:
    m_sim(sim)
    ,m_balance{10,2000}
{

    UpdateOrderBook();
    FindBestBidAsk();
    InitOrders();
}

void SimpleBot::InitOrders() noexcept
{
    const auto bestAskPrice = m_bestAsk.first;
    const auto bestBidPrice = m_bestBid.first;

    const auto amount = - TRADING_POSITION * m_balance.first / MAX_ASK_ORDERS;
    for (auto i=0;i<MAX_ASK_ORDERS;++i)
    {
        auto sellPrice = bestAskPrice * (1 + i * (MAX_PERCENTAGE / MAX_ASK_ORDERS));
        auto result = PlaceOrder(sellPrice, amount);
        if (result)
            m_askOrders.emplace_back(*result);
    }

    auto startingBalance = TRADING_POSITION * m_balance.second;
    for (auto i = 0; i < MAX_BID_ORDERS; ++i)
    {
        auto buyPrice = bestBidPrice * (1 - i * (MAX_PERCENTAGE / MAX_BID_ORDERS));
        auto amount = buyPrice / (startingBalance / MAX_BID_ORDERS);
        auto result = PlaceOrder(buyPrice, amount);
        if (result)
            m_bidOrders.emplace_back(*result);
    }

    m_bidOrdersEnd = m_bidOrders.end();
    m_askOrdersEnd = m_askOrders.end();

}
void SimpleBot::Update() noexcept
{
    UpdateOrderBook();
    FindBestBidAsk();
    //PrintDebugInfo();
    ClearFilledAndCancelOutdated();
    CreateOrders();
    //PrintDebugInfo();
}

std::optional<SimpleBot::Order> SimpleBot::PlaceOrder(Price price, double amount) noexcept
{
    assert(price > 0);
    const auto usdValue = price * amount;
    if (amount > 0 && m_balance.second < usdValue)
        return {};

    if (amount < 0 && m_balance.first < -amount)
        return {};


    auto orderId = m_sim->PlaceOrder(price, amount);

    Order order;

    if (orderId)
    {
        if (amount > 0)
            m_balance.second -= usdValue;
        else
            m_balance.first += amount; 


        order.orderId = *orderId;
        order.price = price;
        order.quantity = amount;

        return order;
    }


    return {};
}

void SimpleBot::UpdateOrderBook() noexcept
{
    m_ob = m_sim->GetOrderBook();
}

void SimpleBot::FindBestBidAsk() noexcept
{
    //sort by price
    std::sort(m_ob.begin(), m_ob.end(), [](OrderBookEntry const& oe1, OrderBookEntry const& oe2) -> bool
        {return oe1.first < oe2.first; });

    //find where ask and bids meet 
    auto bestAsk = std::upper_bound(m_ob.begin(), m_ob.end(), 0, [](double q, OrderBookEntry const& oe) -> bool
        {return oe.second < q; });

    decltype(bestAsk) bestBid;
    bool exit = false;
    uint32_t counter = 0;

    //extra precaution: ignore bids of value 0
    do {
        bestBid = bestAsk - ++counter;
        exit = bestBid->first > 0 && bestBid != m_ob.begin();
    } while (!exit);


    m_bestAsk = *bestAsk;
    m_bestBid = *bestBid;

}

void SimpleBot::ClearFilledAndCancelOutdated() noexcept
{

    const auto bestAskPrice = m_bestAsk.first;
    const auto bestBidPrice = m_bestBid.first;
    const auto minBid = (1. - MAX_PERCENTAGE) * bestBidPrice;
    const auto maxAsk = (1. + MAX_PERCENTAGE) * bestAskPrice;


    m_askOrdersEnd = std::remove_if(m_askOrders.begin(), m_askOrdersEnd, [this,bestAskPrice,maxAsk](Order& o) {

        bool priceAbove = bestAskPrice > o.price;
        bool orderCancelled = false;

        //if price is above order ask price -> it's filled, update balance
        if (priceAbove)
        {
            std::cout << "[BOT] Filled " << o.orderId << "@" << o.price << " ASK" << std::endl;
            m_balance.second -= o.quantity * o.price;
        }
        //if the price is above 5% of the bestAsk, remove it
        else if (o.price > maxAsk)
        {
            orderCancelled = this->CancelOrder(o);
        }


        return (priceAbove | orderCancelled);
        });


    m_bidOrdersEnd = std::remove_if(m_bidOrders.begin(), m_bidOrdersEnd, [this, bestBidPrice, minBid](Order&  o) {
        bool priceBelow = bestBidPrice < o.price;
        bool orderCancelled = false;

        //if price is above order ask price -> it's filled, update balance
        if (priceBelow)
        {
            std::cout << "[BOT] Filled " << o.orderId << "@" << o.price << " BID" << std::endl;
            m_balance.first += o.quantity;

        }
        //if the price is below 5% of the bestAsk, remove it
        else if (o.price < minBid)
            orderCancelled = this->CancelOrder(o);


        return priceBelow | orderCancelled;
        });



}

void SimpleBot::ShowBalances() noexcept
{
    std::cout << std::endl << std::endl << "[BOT]  ------------------------BALANCES---------------------------------------" << std::endl;


    auto eth = m_balance.first;
    auto usd = m_balance.second;
    for (const auto& n : m_bidOrders)
        usd += n.price * n.quantity;

    for (const auto& n : m_askOrders)
        eth -= n.quantity;

    std::cout << "[BOT]  Balance: " << eth << " ETH , " << usd << " USD" << std::endl;

    std::cout  << "[BOT]  -----------------------------------------------------------------------" << std::endl << std::endl << std::endl;



}

void SimpleBot::CreateOrders() noexcept
{
    const auto numberOfBidOrders = std::distance(m_bidOrdersEnd, m_bidOrders.end());
    const auto numberOfAskOrders = std::distance(m_askOrdersEnd, m_askOrders.end());


    uint32_t counter = 1;

    if (m_balance.first > 0 && numberOfAskOrders>0)
    {
        const auto amount = TRADING_POSITION * m_balance.first / numberOfAskOrders;
        const auto bestAskPrice = m_bestAsk.first;
        while (m_askOrdersEnd != m_askOrders.end())
        {
            auto sellPrice = bestAskPrice * (1 + counter * (MAX_PERCENTAGE / numberOfAskOrders));
            auto result = PlaceOrder(sellPrice, -amount);

            if (result)
            {
                *m_askOrdersEnd = *result;
                m_askOrdersEnd++;
            }
            else
                assert(false);

            assert(amount != 0);

            ++counter;
        }
    }



    if (m_balance.second && numberOfBidOrders>0)
    {
        counter = 1;
        auto startingBalance = TRADING_POSITION * m_balance.second;
        const auto bestBidPrice = m_bestBid.first;
        while (m_bidOrdersEnd != m_bidOrders.end())
        {
            auto buyPrice = bestBidPrice * (1 - counter * (MAX_PERCENTAGE / numberOfBidOrders));
            auto amount = (startingBalance / numberOfBidOrders)/buyPrice;
            assert(buyPrice != 0);
            auto result = PlaceOrder(buyPrice, amount);

            if (result)
            {
                *m_bidOrdersEnd = *result;
                m_bidOrdersEnd++;
            }
            else
                assert(false);

            assert(amount != 0);

            ++counter;
        }
    }

    assert(m_balance.second >= 0);
}
bool SimpleBot::CancelOrder(Order& order) noexcept
{
    auto result = m_sim->CancelOrder(order.orderId);

    //Return balances
    if (order.quantity < 0)
        m_balance.first -= order.quantity;
    else
        m_balance.second += order.price*order.quantity;


    return result;
}
void SimpleBot::PrintDebugInfo() noexcept
{
    std::cout << "[BOT]          --- SIMPLE BOT DEBUG INFO ----" << std::endl;
    std::cout << "[BOT]   Balance: " << m_balance.first << " ETH   " << m_balance.second << "    USD " << std::endl;
    std::cout << "[BOT]   Best bid: " << m_bestBid.first << " USD   " << m_bestAsk.first  << "    USD " << std::endl;
    std::cout << "[BOT]  Bid orders:" << std::endl << std::endl;


    for (auto it = m_bidOrders.begin(); it < m_bidOrders.end(); ++it)
        it->PrintDebugInfo();

    for (auto it = m_askOrders.begin(); it < m_askOrders.end(); ++it)
        it->PrintDebugInfo();


    for (auto it = m_ob.begin(); it < m_ob.end(); ++it)
        std::cout << "[BOT] " << it->first << " " << it->second << std::endl;

    std::cout << "[BOT]          ------------------------------" << std::endl;
}
