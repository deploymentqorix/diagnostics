#pragma once
#include "IDebouncer.hpp"
#include <mutex>
#include <chrono>

namespace qcsidm {

class DebouncerTime : public IDebouncer {
public:
    struct Config {
        std::chrono::milliseconds timeAssert{100};
        std::chrono::milliseconds timeClear{100};
    };
    explicit DebouncerTime(const Config& cfg);
    ~DebouncerTime() override = default;
    void sample(bool active) override;
    void setCallback(Callback cb) override;
private:
    Config cfg_;
    std::mutex mx_;
    bool state_{false};
    std::chrono::steady_clock::time_point activeSince_{};
    std::chrono::steady_clock::time_point lastChange_{};
    Callback cb_;
};

} // namespace qcsidm