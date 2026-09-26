#include "Document.h"

Document::Document(const std::string& partNumber,
                    const std::string& filename,
                    const std::string& revision)
    : partNumber_(partNumber),
      filename_(filename),
      revision_(revision),
      state_(DocState::AVAILABLE) {}

void Document::checkOut(const std::string& user) {
    state_ = DocState::CHECKED_OUT;
    checkedOutBy_ = user;
}

void Document::checkIn(bool bumpRevision) {
    if (bumpRevision) {
        revision_ = nextRevision(revision_);
    }
    state_ = DocState::AVAILABLE;
    checkedOutBy_.clear();
}

std::string Document::nextRevision(const std::string& rev) {
    // Simple scheme: single uppercase letter A-Z, wraps into AA style
    // only if you push past Z (kept minimal for this demo).
    if (rev.empty()) return "A";
    std::string next = rev;
    int i = static_cast<int>(next.size()) - 1;
    while (i >= 0) {
        if (next[i] != 'Z') {
            next[i] = static_cast<char>(next[i] + 1);
            return next;
        }
        next[i] = 'A';
        --i;
    }
    return "A" + next; // overflowed past 'ZZZ...' -> grow width
}

std::string Document::toString() const {
    std::string s = "[" + partNumber_ + "] " + filename_ +
                     " (rev " + revision_ + ") - ";
    s += (state_ == DocState::AVAILABLE) ? "AVAILABLE" : "CHECKED_OUT by " + checkedOutBy_;
    return s;
}
