#include "VaultManager.h"
#include <iostream>
#include <stdexcept>

void VaultManager::addDocument(const std::string& partNumber,
                                const std::string& filename,
                                const std::string& revision) {
    std::lock_guard<std::mutex> lock(vaultMutex_);
    vault_[partNumber] = Document(partNumber, filename, revision);
}

bool VaultManager::checkOut(const std::string& partNumber, const std::string& user) {
    std::lock_guard<std::mutex> lock(vaultMutex_);

    auto it = vault_.find(partNumber);
    if (it == vault_.end()) {
        auditLog_.record(user, "CHECK_OUT_FAILED", partNumber, "no such document");
        return false;
    }

    Document& doc = it->second;
    if (doc.getState() == DocState::CHECKED_OUT) {
        // This is the core PLM guarantee: reject the second writer
        // instead of allowing a silent overwrite.
        auditLog_.record(user, "CHECK_OUT_DENIED", partNumber,
                          "already locked by " + doc.getCheckedOutBy());
        return false;
    }

    doc.checkOut(user);
    auditLog_.record(user, "CHECK_OUT", partNumber, "rev " + doc.getRevision());
    return true;
}

bool VaultManager::checkIn(const std::string& partNumber, const std::string& user,
                            bool bumpRevision) {
    std::lock_guard<std::mutex> lock(vaultMutex_);

    auto it = vault_.find(partNumber);
    if (it == vault_.end()) {
        auditLog_.record(user, "CHECK_IN_FAILED", partNumber, "no such document");
        return false;
    }

    Document& doc = it->second;
    if (doc.getState() != DocState::CHECKED_OUT) {
        auditLog_.record(user, "CHECK_IN_FAILED", partNumber, "was not checked out");
        return false;
    }
    if (doc.getCheckedOutBy() != user) {
        auditLog_.record(user, "CHECK_IN_DENIED", partNumber,
                          "locked by " + doc.getCheckedOutBy() + ", not " + user);
        return false;
    }

    std::string oldRev = doc.getRevision();
    doc.checkIn(bumpRevision);
    auditLog_.record(user, "CHECK_IN", partNumber,
                      oldRev + " -> " + doc.getRevision());
    return true;
}

void VaultManager::printVaultStatus() const {
    std::lock_guard<std::mutex> lock(vaultMutex_);
    std::cout << "\n--- Vault Status (" << vault_.size() << " documents) ---\n";
    for (const auto& [partNumber, doc] : vault_) {
        std::cout << "  " << doc.toString() << "\n";
    }
}

void VaultManager::printHistory(const std::string& partNumber) const {
    auto history = auditLog_.historyFor(partNumber);
    std::cout << "\n--- History for " << partNumber << " (" << history.size() << " events) ---\n";
    for (const auto& e : history) {
        std::cout << "[" << e.timestamp << "] " << e.user << " -> " << e.action;
        if (!e.details.empty()) std::cout << " (" << e.details << ")";
        std::cout << "\n";
    }
}
