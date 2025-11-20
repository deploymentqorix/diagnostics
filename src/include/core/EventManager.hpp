#pragma once
#include "IEventManager.hpp"
#include "IDtcManager.hpp"
#include "DiagnosticWorker.hpp"
#include <unordered_map>
#include <memory>
#include <mutex>

namespace qcsidm {

class EventManager : public IEventManager {
public:
    EventManager(std::shared_ptr<IDtcManager> dtcManager,
                 std::shared_ptr<IDiagnosticWorker> worker);

    void onDebouncedSample(EventId id, bool asserted) override;
    void registerEvent(const EventConfig& cfg, std::shared_ptr<IDebouncer> debouncer) override;

private:
    std::mutex mx_;
    std::unordered_map<EventId, EventConfig> configs_;
    std::unordered_map<EventId, std::shared_ptr<IDebouncer>> debouncers_;
    std::shared_ptr<IDtcManager> dtcManager_;
    std::shared_ptr<IDiagnosticWorker> worker_;
};

} // namespace qcsidm