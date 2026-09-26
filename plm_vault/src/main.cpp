#include <iostream>
#include <thread>
#include <chrono>
#include "VaultManager.h"

// Simulates one engineer's session: try to check out a part, "edit" it
// for a bit, then check it back in. If the check-out is denied (because
// someone else already holds the lock), the engineer backs off and
// retries after a short delay -- exactly how a real vault client behaves.
void engineerSession(VaultManager& vault, const std::string& user,
                      const std::string& partNumber, int editMillis) {
    const int maxRetries = 5;
    for (int attempt = 1; attempt <= maxRetries; ++attempt) {
        if (vault.checkOut(partNumber, user)) {
            std::cout << user << " checked out " << partNumber
                      << " (attempt " << attempt << ")\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(editMillis));
            vault.checkIn(partNumber, user, /*bumpRevision=*/true);
            std::cout << user << " checked " << partNumber << " back in\n";
            return;
        }
        std::cout << user << " could NOT check out " << partNumber
                   << " (locked) - retrying...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::cout << user << " gave up on " << partNumber << " after "
               << maxRetries << " attempts\n";
}

int main() {
    VaultManager vault;

    vault.addDocument("PN-1001", "bracket_v1.prt", "A");
    vault.addDocument("PN-1002", "housing_v1.prt", "A");

    std::cout << "=== Initial vault state ===\n";
    vault.printVaultStatus();

    std::cout << "\n=== Simulating two engineers racing for PN-1001 ===\n";
    // Alice and Bob both try to check out the SAME document at the same
    // time. The VaultManager's lock guarantees only one wins; the other
    // is denied and must retry -- this is the behavior we're proving out.
    std::thread t1(engineerSession, std::ref(vault), "alice", "PN-1001", 150);
    std::thread t2(engineerSession, std::ref(vault), "bob",   "PN-1001", 100);

    // Meanwhile a third engineer works on an unrelated part concurrently,
    // with no contention at all.
    std::thread t3(engineerSession, std::ref(vault), "carol", "PN-1002", 80);

    t1.join();
    t2.join();
    t3.join();

    std::cout << "\n=== Final vault state ===\n";
    vault.printVaultStatus();

    vault.printHistory("PN-1001");
    vault.printHistory("PN-1002");

    return 0;
}
