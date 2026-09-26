#include "AuditLog.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>

std::string AuditLog::currentTimestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t t = system_clock::to_time_t(now);
    std::tm tmBuf{};
#if defined(_WIN32)
    localtime_s(&tmBuf, &t);
#else
    localtime_r(&t, &tmBuf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmBuf, "%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

void AuditLog::record(const std::string& user,
                       const std::string& action,
                       const std::string& partNumber,
                       const std::string& details) {
    std::lock_guard<std::mutex> lock(mutex_);
    entries_.push_back(AuditEntry{currentTimestamp(), user, action, partNumber, details});
}

void AuditLog::printAll() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "\n--- Audit Log (" << entries_.size() << " entries) ---\n";
    for (const auto& e : entries_) {
        std::cout << "[" << e.timestamp << "] " << e.user
                  << " -> " << e.action << " " << e.partNumber;
        if (!e.details.empty()) std::cout << " (" << e.details << ")";
        std::cout << "\n";
    }
}

std::vector<AuditEntry> AuditLog::historyFor(const std::string& partNumber) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<AuditEntry> result;
    for (const auto& e : entries_) {
        if (e.partNumber == partNumber) result.push_back(e);
    }
    return result;
}
