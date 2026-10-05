#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace ome {

enum class CommandType {
    PlaceLimit,
    CancelOrder,
    OpenSession,
    CloseSession,
};

struct Command {
    CommandType type{};
    std::string trader_id;
    std::string order_id;
    std::string side;
    std::optional<std::int64_t> price;
    std::optional<std::int64_t> quantity;
};

struct SequencedCommand {
    std::uint64_t sequence{};
    Command command;
    std::string status;
};

std::string to_string(CommandType type);

}  // namespace ome
