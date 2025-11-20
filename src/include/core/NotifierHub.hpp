#pragma once
#include "INotifier.hpp"
#include <vector>
#include <mutex>
#include <memory>
#include <functional>

namespace qcsidm {

class NotifierHub : public INotifierHub, public std::enable_shared_from_this<NotifierHub> {
public:
    NotifierHub() = default;
    ~NotifierHub() override = default;
    void registerNotifier(std::weak_ptr<INotifier> n) override;
    void notifyDtcCreated(std::uint32_t dtcId) override;
    void notifyDtcCleared(std::uint32_t dtcId) override;
    void notifyOverflowOccur(std::uint32_t count) override;
    void notifyOverflowClear() override;
private:
    void dispatchToSubs(const std::function<void(std::shared_ptr<INotifier>)>& f);
    std::mutex mx_;
    std::vector<std::weak_ptr<INotifier>> subs_;
};

} // namespace qcsidm
