# PLM Document Vault Simulator (C++)

A small C++ simulation of the check-in / check-out engine at the heart of
PLM systems like Siemens Teamcenter: it guarantees that two engineers can
never silently overwrite each other's work on the same document, and it
keeps an immutable audit trail of every event.

## Why this project

Most student projects touch CAD or scripting. This one models the piece
of PLM that's easy to overlook but is actually the core value
proposition of a vault: **concurrency control over engineering data**.
It demonstrates:

- **OOP design** — `Document`, `AuditLog`, `VaultManager` each own a
  single responsibility.
- **Thread safety** — `std::mutex` protects the vault from race
  conditions when multiple "engineers" (threads) act at once.
- **Real PLM semantics** — check-out locks a document; check-in
  releases it and bumps the revision (A → B → C…); a second check-out
  attempt on a locked document is denied, not queued or silently
  allowed.
- **Traceability** — the `AuditLog` is append-only by design (no
  edit/delete method exists), mirroring how PLM systems treat history
  as immutable for compliance reasons.

## Project structure

```
plm_vault/
├── include/
│   ├── Document.h       # a single document's identity, revision, lock state
│   ├── AuditLog.h       # append-only event history
│   └── VaultManager.h   # the vault: owns documents, enforces locking
├── src/
│   ├── Document.cpp
│   ├── AuditLog.cpp
│   ├── VaultManager.cpp
│   └── main.cpp         # demo: two engineers race for the same part
├── CMakeLists.txt
└── README.md
```

## Building and running

With CMake:

```bash
mkdir build && cd build
cmake ..
cmake --build .
./plm_vault
```

Or directly with g++ (no CMake required):

```bash
g++ -std=c++17 -Wall -Wextra -pthread -Iinclude src/*.cpp -o plm_vault_demo
./plm_vault_demo
```

## What the demo shows

`main.cpp` spins up three threads simulating engineers:

- **Alice** and **Bob** both try to check out the *same* part
  (`PN-1001`) at nearly the same instant.
- **Carol** works on a different, unrelated part (`PN-1002`) with no
  contention.

The output shows Bob getting denied and retrying until Alice checks the
part back in, at which point Bob succeeds — proving the lock actually
prevents the double-edit scenario rather than just describing it. The
printed audit log gives a full, timestamped history per part.

## Ways to extend it (good interview talking points)

- **Queueing instead of retry/backoff** — hold a wait-list per document
  so the next requester is notified instead of polling.
- **Multi-site locking** — simulate two "sites" with a replication
  delay, and discuss how real PLM systems handle distributed locks.
- **Persistence** — serialize the vault + audit log to disk (JSON) so
  state survives a restart.
- **Where-used / dependency checks** — block check-out of a part if a
  parent assembly is already checked out by someone else, to avoid
  inconsistent BOM states.
- **Role-based permissions** — only certain users can check in a
  document to a "Released" state.
