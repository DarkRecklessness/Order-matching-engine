#pragma once

#include "ome/sequencer.hpp"

#include <string>

namespace ome {

struct ApiResponse {
    int status_code{};
    std::string body;
};

// Не зависящий от транспорта слой API. HTTP-адаптер только преобразует запросы;
// валидация и назначение номеров выполняются здесь и тестируются без сокетов.
class CommandApi {
public:
    ApiResponse health() const;
    ApiResponse sequencer_state() const;
    ApiResponse submit_command(const std::string& json_body);

private:
    CommandSequencer sequencer_;
};

}  // namespace ome
