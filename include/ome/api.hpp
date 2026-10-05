#pragma once

#include "ome/sequencer.hpp"

#include <string>

namespace ome {

struct ApiResponse {
    int status_code{};
    std::string body;
};

// Прикладной слой текущего JSON-прототипа отделён от HTTP-маршрутизации:
// валидация и назначение номеров тестируются без сокетов. Целевой бинарный
// TCP gateway и его декодер в эту техническую основу пока не входят.
class CommandApi {
public:
    ApiResponse health() const;
    ApiResponse sequencer_state() const;
    ApiResponse submit_command(const std::string& json_body);

private:
    CommandSequencer sequencer_;
};

}  // namespace ome
