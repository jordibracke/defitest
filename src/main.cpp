#include "DvfSimulator.h"
#include "Bot.h"
#include <algorithm>
#include <memory>
#include <chrono>
#include <thread>



int main()
{

    constexpr auto UPDATE_PERIOD_MS = 5000;
    auto sim = DvfSimulator::Create();

    SimpleBot b(sim);
    bool exitCondition = false;
    while (!exitCondition)
    {
        auto start = std::chrono::high_resolution_clock::now();
        static uint8_t counter = 0;

        b.Update();

        bool showBalances = --counter % 6 == 0;
        if (showBalances)
        {
            b.ShowBalances();
            counter = 0;
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_PERIOD_MS - elapsed.count()));
    }


    return true;
}