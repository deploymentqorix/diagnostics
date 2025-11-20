#pragma once
#include "IDebouncer.hpp"
#include <mutex>

namespace qcsidm {

class DebouncerCounter : public IDebouncer {
public:
    struct Config { std::uint32_t assertThreshold{3}; std::uint32_t clearThreshold{1}; };
    explicit DebouncerCounter(const Config& cfg);
    void sample(bool active) override;
    void setCallback(Callback cb) override;
private:
    Config cfg_;
    mutable std::mutex mx_;
    std::uint32_t counter_{0};
    bool state_{false};
    Callback cb_;
};

} // namespace qcsidm
