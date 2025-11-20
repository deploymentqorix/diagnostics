#include "../../include/core/DiagnosticWorker.hpp"
#include <iostream>
 
namespace qcsidm {
 
DiagnosticWorker::DiagnosticWorker(WorkerCallback cb, size_t maxQueue)
: cb_(std::move(cb)), maxQueue_(maxQueue) {}
 
DiagnosticWorker::~DiagnosticWorker() {
    stop();
}
 
void DiagnosticWorker::start() {
    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true)) return;
    workerThread_ = std::thread(&DiagnosticWorker::run, this);
}
 
void DiagnosticWorker::stop() {
    bool expected = true;
    if (!running_.compare_exchange_strong(expected, false)) return;
    cv_.notify_all();
    if (workerThread_.joinable()) workerThread_.join();
}
 
void DiagnosticWorker::enqueue(const WorkerEvent& ev) {
    std::unique_lock<std::mutex> lk(mx_);
    // bounded queue policy: drop oldest when full
    if (queue_.size() >= maxQueue_) {
        queue_.pop_front(); // drop oldest
    }
    queue_.push_back(ev);
    lk.unlock();
    cv_.notify_one();
}
 
void DiagnosticWorker::run() {
    while (running_.load()) {
        WorkerEvent ev;
        {
            std::unique_lock<std::mutex> lk(mx_);
            cv_.wait(lk, [&]{ return !queue_.empty() || !running_.load(); });
            if (!running_.load() && queue_.empty()) break;
            ev = queue_.front();
            queue_.pop_front();
        }
        try {
            cb_(ev);
        } catch (const std::exception& ex) {
            std::cerr << "[DiagnosticWorker] handler exception: " << ex.what() << "\n";
        } catch (...) {
            std::cerr << "[DiagnosticWorker] handler unknown exception\n";
        }
    }
}
 
} // namespace qcsidm