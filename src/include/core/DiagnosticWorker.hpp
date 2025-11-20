#pragma once
#include "types.hpp"
#include <functional>
#include <thread>
#include <atomic>
#include <deque>
#include <mutex>
#include <condition_variable>

namespace qcsidm {

struct WorkerEvent {
    EventId eventId;
    bool asserted;
    std::chrono::system_clock::time_point timestamp;
};

class IDiagnosticWorker {
public:
    virtual ~IDiagnosticWorker() = default;
    virtual void enqueue(const WorkerEvent& ev) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

class DiagnosticWorker : public IDiagnosticWorker {
public:
    using WorkerCallback = std::function<void(const WorkerEvent&)>;
    DiagnosticWorker(WorkerCallback cb, size_t maxQueue = 1024);
    ~DiagnosticWorker();
    void enqueue(const WorkerEvent& ev) override;
    void start() override;
    void stop() override;
private:
    void run();
    WorkerCallback cb_;
    size_t maxQueue_;
    std::thread workerThread_;
    std::atomic<bool> running_{false};
    mutable std::mutex mx_;
    std::condition_variable cv_;
    std::deque<WorkerEvent> queue_;
};

} // namespace qcsidm
