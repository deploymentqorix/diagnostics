#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <ctime>

#include "core/NotifierHub.hpp"
#include "core/EventMemory.hpp"
#include "core/FilePersistence.hpp"
#include "core/DtcManager.hpp"
#include "core/EventManager.hpp"
#include "core/DebouncerCounter.hpp"
#include "core/DebouncerTime.hpp"
#include "core/DiagnosticWorker.hpp"

using namespace qcsidm;

class ConsoleNotifier : public INotifier, public std::enable_shared_from_this<ConsoleNotifier> {
public:
    void onDtcCreated(std::uint32_t dtcId) override {
        std::cout << "[NOTIFY] DTC Created: " << std::hex << dtcId << std::dec << std::endl;
    }
    void onDtcCleared(std::uint32_t dtcId) override {
        std::cout << "[NOTIFY] DTC Cleared: " << std::hex << dtcId << std::dec << std::endl;
    }
    void onOverflowOccur(std::uint32_t count) override {
        std::cout << "[NOTIFY] Overflow Occurred: " << count << std::endl;
    }
    void onOverflowClear() override {
        std::cout << "[NOTIFY] Overflow Cleared" << std::endl;
    }
};

int main() {
    // set up core components
    auto notifierHub = std::make_shared<NotifierHub>();
    auto consoleNotifier = std::make_shared<ConsoleNotifier>();
    notifierHub->registerNotifier(consoleNotifier);

    auto persistence = std::make_shared<FilePersistence>("diagnostics_store.csv");
    auto eventMemory = std::make_shared<EventMemory>(10, notifierHub);
    auto dtcManager = std::make_shared<qcsidm::DtcManager>(
        eventMemory,
        std::static_pointer_cast<qcsidm::INotifierHub>(notifierHub),
        std::static_pointer_cast<qcsidm::IPersistence>(persistence)
    );

    // Diagnostic worker forwards events to DtcManager
    auto diagWorker = std::make_shared<DiagnosticWorker>(
        [dtcManager](const WorkerEvent& ev) {
            if (dtcManager) dtcManager->handleEventChange(ev.eventId, ev.asserted);
        },
        1024
    );
    diagWorker->start();

    auto eventManager = std::make_shared<EventManager>(
        std::static_pointer_cast<qcsidm::IDtcManager>(dtcManager),
        std::static_pointer_cast<qcsidm::IDiagnosticWorker>(diagWorker)
    );

    // register counter-based debouncer
    auto debCounter = std::make_shared<DebouncerCounter>(DebouncerCounter::Config{3,1});
    EventConfig engineOverheatCfg;
    engineOverheatCfg.id = 1;
    engineOverheatCfg.mappedDtc = 0x1001;
    engineOverheatCfg.name = "EngineOverheat";
    eventManager->registerEvent(engineOverheatCfg, debCounter);

    // register time-based debouncer
    DebouncerTime::Config tcfg;
    tcfg.timeAssert = std::chrono::milliseconds(150);
    tcfg.timeClear  = std::chrono::milliseconds(150);
    auto debTime = std::make_shared<DebouncerTime>(tcfg);
    EventConfig regenFaultCfg;
    regenFaultCfg.id = 2;
    regenFaultCfg.mappedDtc = 0x1002;
    regenFaultCfg.name = "RegenFault";
    eventManager->registerEvent(regenFaultCfg, debTime);

    // simulate counter-based event (should assert)
    std::cout << "Simulating EngineOverheat (counter-based)..." << std::endl;
    for (int i = 0; i < 5; ++i) { debCounter->sample(true); std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    for (int i = 0; i < 5; ++i) { debCounter->sample(false); std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // simulate time-based event (short spike -> filtered)
    std::cout << "Simulating RegenFault (short spike)..." << std::endl;
    debTime->sample(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    debTime->sample(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // sustained time-based event (should assert)
    std::cout << "Simulating RegenFault (sustained)..." << std::endl;
    debTime->sample(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    debTime->sample(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // generate many events quickly to force EventMemory overflow
    std::cout << "Creating many ephemeral events to trigger overflow..." << std::endl;
    for (int eid = 3; eid < 30; ++eid) {
        auto db = std::make_shared<DebouncerCounter>(DebouncerCounter::Config{1,0});
        EventConfig ec;
        ec.id = static_cast<EventId>(eid);
        ec.mappedDtc = 0x2000 + eid;
        ec.name = "Ev" + std::to_string(eid);
        eventManager->registerEvent(ec, db);
        db->sample(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Active event memory size: " << eventMemory->size() << std::endl;

    // try to load persisted DTCs
    std::vector<DtcRecord> persisted;
    if (persistence->load(persisted)) {
        std::cout << "Persisted DTCs: " << persisted.size() << std::endl;
    } else {
        std::cout << "No persisted records or failed to load persistence." << std::endl;
    }

    // shutdown
    diagWorker->stop();
    std::cout << "Shutdown complete." << std::endl;
    return 0;
}
