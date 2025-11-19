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
                 std::shared_ptr<IDiagnosticWorker> worker)
      : dtcManager_(std::move(dtcManager)), worker_(std::move(worker)) {}

    void onDebouncedSample(EventId id, bool asserted) override {
        // enqueue to worker if present, otherwise direct call
        if (worker_) {
            WorkerEvent ev; ev.eventId = id; ev.asserted = asserted; ev.timestamp = std::chrono::system_clock::now();
            worker_->enqueue(ev);
        } else if (dtcManager_) {
            dtcManager_->handleEventChange(id, asserted);
        }
    }

    void registerEvent(const EventConfig& cfg, std::shared_ptr<IDebouncer> debouncer) override {
        std::lock_guard<std::mutex> lk(mx_);
        configs_[cfg.id] = cfg;
        debouncers_[cfg.id] = debouncer;
        // set callback to call onDebouncedSample
        if (debouncer) {
            debouncer->setCallback([this, id = cfg.id](bool asserted) {
                this->onDebouncedSample(id, asserted);
            });
        }
    }

private:
    std::mutex mx_;
    std::unordered_map<EventId, EventConfig> configs_;
    std::unordered_map<EventId, std::shared_ptr<IDebouncer>> debouncers_;
    std::shared_ptr<IDtcManager> dtcManager_;
    std::shared_ptr<IDiagnosticWorker> worker_;
};

} // namespace qcsidm
