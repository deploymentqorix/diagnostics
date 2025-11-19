#pragma once
#include <cstdint>
#include <memory>

namespace qcsidm {

class INotifier {
public:
    virtual ~INotifier() = default;
    virtual void onDtcCreated(std::uint32_t dtcId) = 0;
    virtual void onDtcCleared(std::uint32_t dtcId) = 0;
    virtual void onOverflowOccur(std::uint32_t count) = 0;
    virtual void onOverflowClear() = 0;
};

class INotifierHub {
public:
    virtual ~INotifierHub() = default;
    virtual void registerNotifier(std::weak_ptr<INotifier> n) = 0;
    virtual void notifyDtcCreated(std::uint32_t dtcId) = 0;
    virtual void notifyDtcCleared(std::uint32_t dtcId) = 0;
    virtual void notifyOverflowOccur(std::uint32_t count) = 0;
    virtual void notifyOverflowClear() = 0;
};

} // namespace qcsidm
