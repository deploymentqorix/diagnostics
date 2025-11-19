#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include "core/DebouncerTime.hpp"

using namespace qcsidm;

int main() {
    DebouncerTime::Config cfg;
    cfg.timeAssert = std::chrono::milliseconds(150);
    cfg.timeClear  = std::chrono::milliseconds(150);
    auto deb = std::make_shared<DebouncerTime>(cfg);

    deb->setCallback([&](bool asserted){
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if (asserted) std::cout << "[HARNESS] Debouncer ASSERTED at " << std::ctime(&now);
        else          std::cout << "[HARNESS] Debouncer CLEARED at " << std::ctime(&now);
    });

    std::cout << "HARNESS: Short spike (should NOT assert)\n";
    deb->sample(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(70));
    deb->sample(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::cout << "HARNESS: Sustained active (should assert)\n";
    deb->sample(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    deb->sample(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::cout << "HARNESS: End\n";
    return 0;
}
