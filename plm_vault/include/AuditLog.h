#pragma once
#include <string>
#include <vector>
#include <mutex>

// One immutable record of something that happened to a document.
struct AuditEntry {
    std::string timestamp;
    std::string user;
    std::string action;      // e.g. "CHECK_OUT", "CHECK_IN", "CHECK_OUT_DENIED"
    std::string partNumber;
    std::string details;
};

// Thread-safe, append-only audit trail. Real PLM systems never let you
// edit or delete history -- you can only add to it -- so there is
// deliberately no "remove" or "edit" method here.
class AuditLog {
public:
    void record(const std::string& user,
                const std::string& action,
                const std::string& partNumber,
                const std::string& details = "");

    void printAll() const;

    std::vector<AuditEntry> historyFor(const std::string& partNumber) const;

private:
    mutable std::mutex mutex_;
    std::vector<AuditEntry> entries_;

    static std::string currentTimestamp();
};
