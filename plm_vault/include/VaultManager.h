#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include "Document.h"
#include "AuditLog.h"

// Central vault: owns all documents, enforces the single-writer lock
// (only one user may hold a document checked out at a time), and
// records every state change to the audit log.
//
// This is the piece that mirrors what Teamcenter's vault/document
// management actually guarantees: two engineers can never silently
// overwrite each other's work on the same file.
class VaultManager {
public:
    void addDocument(const std::string& partNumber,
                      const std::string& filename,
                      const std::string& revision = "A");

    // Returns true if the check-out succeeded. Returns false (and logs
    // a denial) if the document is already locked by someone else.
    bool checkOut(const std::string& partNumber, const std::string& user);

    // Returns true if the check-in succeeded. Fails if the document
    // isn't checked out, or is checked out by a different user.
    bool checkIn(const std::string& partNumber, const std::string& user,
                 bool bumpRevision = true);

    void printVaultStatus() const;
    void printHistory(const std::string& partNumber) const;

private:
    mutable std::mutex vaultMutex_;
    std::unordered_map<std::string, Document> vault_;
    AuditLog auditLog_;
};
