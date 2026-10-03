#pragma once

#include "ome/command.hpp"

#include <cstdint>
#include <mutex>
#include <vector>

namespace ome {

// Присваивает каждой принятой команде единый монотонно возрастающий номер.
// submit() является точкой сериализации. Позже здесь будет добавлена запись в WAL
// перед передачей команды в matching engine.
class CommandSequencer {
public:
    SequencedCommand submit(Command command);
    [[nodiscard]] std::uint64_t last_sequence() const;
    [[nodiscard]] std::vector<SequencedCommand> history() const;

private:
    mutable std::mutex mutex_;
    std::uint64_t last_sequence_{0};
    std::vector<SequencedCommand> history_;
};

}  // namespace ome
