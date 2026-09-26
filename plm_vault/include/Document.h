#pragma once
#include <string>

// Lifecycle-relevant state of a document in the vault.
enum class DocState {
    AVAILABLE,     // checked in, free to check out
    CHECKED_OUT    // locked by a user for editing
};

// Represents a single engineering document (e.g. a CAD file) tracked
// by the vault: its identity, current revision, and lock state.
class Document {
public:
    Document() = default;
    Document(const std::string& partNumber,
              const std::string& filename,
              const std::string& revision);

    const std::string& getPartNumber() const { return partNumber_; }
    const std::string& getFilename()   const { return filename_; }
    const std::string& getRevision()   const { return revision_; }
    DocState           getState()      const { return state_; }
    const std::string& getCheckedOutBy() const { return checkedOutBy_; }

    // Locks the document to `user`. Caller must have already verified
    // it was AVAILABLE; this just performs the state transition.
    void checkOut(const std::string& user);

    // Unlocks the document. If bumpRevision is true, advances the
    // revision letter (A -> B -> C ...) to represent a new version.
    void checkIn(bool bumpRevision);

    std::string toString() const;

private:
    std::string partNumber_;
    std::string filename_;
    std::string revision_ = "A";
    DocState state_ = DocState::AVAILABLE;
    std::string checkedOutBy_;

    static std::string nextRevision(const std::string& rev);
};
