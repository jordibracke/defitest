
#include <iostream>
#include <vector>
#include "DvfSimulator.h"

class IBot
{
public:
    
    using Price = double;
    using Balance = std::pair<double, double>;
    using OrderBookEntry = std::pair<double, double>;

    struct Order {
        Price price{ 0 };
        double quantity{ 0 };
        IDvfSimulator::OrderID orderId{ 0 };
        void PrintDebugInfo() {
            std::cout << "[BOT]  OrderId: " << orderId << " Price: " << price << " Quantity: " << quantity << std::endl;
        }
    };
    // Updates the bot
    virtual void Update() noexcept = 0;
    virtual void ShowBalances() noexcept = 0;

};

class SimpleBot : public IBot
{
public:
    static constexpr double MAX_PERCENTAGE = 0.05;
    static constexpr uint32_t MAX_BID_ORDERS = 5;
    static constexpr uint32_t MAX_ASK_ORDERS = 5;
    static constexpr double TRADING_POSITION = 0.5;

    virtual ~SimpleBot() noexcept = default;
    SimpleBot(IDvfSimulator* sim) noexcept;
    void Update() noexcept override final;
    void ShowBalances() noexcept override final;


    Balance GetBalance() noexcept { return m_balance; };
    IDvfSimulator::OrderBook GetOrderBook() noexcept { return m_ob; };
    OrderBookEntry GetBestBid() noexcept { return m_bestBid; };
    OrderBookEntry GetBestAsk() noexcept { return m_bestAsk; };


private:
    std::optional<Order> PlaceOrder(Price price, double amount) noexcept;
    void InitOrders() noexcept;
    bool CancelOrder(Order& order) noexcept;
    void UpdateOrderBook() noexcept;
    void FindBestBidAsk() noexcept;
    void ClearFilledAndCancelOutdated() noexcept;
    void CreateOrders() noexcept;
    void PrintDebugInfo() noexcept;

    Balance m_balance;
    std::vector<Order> m_bidOrders;
    std::vector<Order>::iterator m_bidOrdersEnd;
    std::vector<Order> m_askOrders;
    std::vector<Order>::iterator m_askOrdersEnd;

    IDvfSimulator::OrderBook m_ob;
    OrderBookEntry m_bestBid;
    OrderBookEntry m_bestAsk;



    IDvfSimulator* m_sim;

};