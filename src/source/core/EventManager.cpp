#include "../../include/core/EventManager.hpp"
#include "../../include/core/IDtcManager.hpp"
#include <iostream>
 
namespace qcsidm {
 
EventManager::EventManager(std::shared_ptr<IDtcManager> dtcManager,
                           std::shared_ptr<IDiagnosticWorker> worker)
    : dtcManager_(std::move(dtcManager)), worker_(std::move(worker)) {}
 
void EventManager::registerEvent(const EventConfig& cfg, std::shared_ptr<IDebouncer> debouncer) {
    std::lock_guard<std::mutex> lk(mx_);
    configs_[cfg.id] = cfg;
    debouncers_[cfg.id] = debouncer;
 
    // Attach a *lightweight* callback: enqueue to diagnostic worker
    debouncer->setCallback([this, id = cfg.id](bool asserted) {
        WorkerEvent ev;
        ev.eventId = id;
        ev.asserted = asserted;
        ev.timestamp = std::chrono::system_clock::now();
        if (this->worker_) this->worker_->enqueue(ev);
    });
}
 
void EventManager::onDebouncedSample(EventId id, bool asserted) {
    // legacy: direct forward — prefer using registered debouncer callbacks which enqueues
    if (worker_) {
        WorkerEvent ev;
        ev.eventId = id;
        ev.asserted = asserted;
        ev.timestamp = std::chrono::system_clock::now();
        worker_->enqueue(ev);
    } else {
        // fallback: direct call (rare)
        if (dtcManager_) dtcManager_->handleEventChange(id, asserted);
    }
}
 
} // namespace qcsidm