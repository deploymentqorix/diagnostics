#pragma once
#include <functional>

namespace qcsidm {

class IDebouncer {
public:
    using Callback = std::function<void(bool asserted)>;
    virtual ~IDebouncer() = default;
    virtual void sample(bool active) = 0;
    virtual void setCallback(Callback cb) = 0;
};

} // namespace qcsidm
