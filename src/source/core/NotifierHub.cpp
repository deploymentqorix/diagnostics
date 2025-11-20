#include "../../include/core/NotifierHub.hpp"
#include <algorithm>
 
 
namespace qcsidm {
 
 
void NotifierHub::registerNotifier(std::weak_ptr<INotifier> n) {
std::lock_guard<std::mutex> lk(mx_);
subs_.push_back(n);
}
 
 
void NotifierHub::dispatchToSubs(const std::function<void(std::shared_ptr<INotifier>)>& f) {
    std::vector<std::shared_ptr<INotifier>> targets;
    {
        std::lock_guard<std::mutex> lk(mx_);
        for (auto &w : subs_) {
            if (auto s = w.lock()) targets.push_back(s);
        }
        // compact subs_
        std::vector<std::weak_ptr<INotifier>> compact;
        compact.reserve(targets.size());
        for (auto &s : targets) compact.push_back(std::weak_ptr<INotifier>(s));
        subs_.swap(compact);
    }
    for (auto &s : targets) {
        try { f(s); } catch (...) {}
    }
}
 
void NotifierHub::notifyDtcCreated(std::uint32_t dtcId) {
dispatchToSubs([dtcId](std::shared_ptr<INotifier> s){ s->onDtcCreated(dtcId); });
}
void NotifierHub::notifyDtcCleared(std::uint32_t dtcId) {
dispatchToSubs([dtcId](std::shared_ptr<INotifier> s){ s->onDtcCleared(dtcId); });
}
void NotifierHub::notifyOverflowOccur(std::uint32_t count) {
dispatchToSubs([count](std::shared_ptr<INotifier> s){ s->onOverflowOccur(count); });
}
void NotifierHub::notifyOverflowClear() {
dispatchToSubs([](std::shared_ptr<INotifier> s){ s->onOverflowClear(); });
}
 
 
} // namespace qcsidm