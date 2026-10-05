#include "ome/sequencer.hpp"

#include <utility>

namespace ome {

SequencedCommand CommandSequencer::submit(Command command) {
    std::lock_guard<std::mutex> lock(mutex_);

    SequencedCommand accepted{
        .sequence = ++last_sequence_,
        .command = std::move(command),
        // Matching engine и WAL намеренно не входят в реализацию к EK1.
        .status = "accepted_stub",
    };
    history_.push_back(accepted);
    return accepted;
}

std::uint64_t CommandSequencer::last_sequence() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_sequence_;
}

std::vector<SequencedCommand> CommandSequencer::history() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return history_;
}

}  // namespace ome
