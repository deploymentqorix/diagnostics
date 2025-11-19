#pragma once
#include "types.hpp"
#include "IDebouncer.hpp"
#include <memory>

namespace qcsidm {
class IDtcManager;
class IEventManager {
public:
    virtual ~IEventManager() = default;
    virtual void onDebouncedSample(EventId id, bool asserted) = 0;
    virtual void registerEvent(const EventConfig& cfg, std::shared_ptr<IDebouncer> debouncer) = 0;
};
} // namespace qcsidm
