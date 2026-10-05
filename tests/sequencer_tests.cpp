#include "ome/api.hpp"
#include "ome/sequencer.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

namespace {

void test_integer_validation() {
    ome::CommandApi api;

    const std::vector<std::string> invalid_commands{
        R"({"type":"PLACE_LIMIT","trader_id":"M1","side":"BUY","price":1.5,"quantity":5})",
        R"({"type":"PLACE_LIMIT","trader_id":"M1","side":"BUY","price":9223372036854775808,"quantity":5})",
        R"({"type":"PLACE_LIMIT","trader_id":"M1","side":"BUY","price":100,"quantity":1e3})",
    };

    for (const auto& command : invalid_commands) {
        const auto response = api.submit_command(command);
        assert(response.status_code == 400);
        assert(api.sequencer_state().body == R"({"last_sequence":0})");
    }

    const auto max_int64 = api.submit_command(
        R"({"type":"PLACE_LIMIT","trader_id":"M1","side":"BUY","price":9223372036854775807,"quantity":1})");
    assert(max_int64.status_code == 202);
    assert(max_int64.body.find(R"("sequence":1)") != std::string::npos);
}

void test_validation_and_api_sequence() {
    ome::CommandApi api;

    const auto invalid = api.submit_command(
        R"({"type":"PLACE_LIMIT","trader_id":"M1","side":"BUY","price":0,"quantity":5})");
    assert(invalid.status_code == 400);
    assert(api.sequencer_state().body == R"({"last_sequence":0})");

    const auto first = api.submit_command(
        R"({"type":"PLACE_LIMIT","trader_id":"M1","side":"BUY","price":100,"quantity":5})");
    const auto second = api.submit_command(
        R"({"type":"CANCEL_ORDER","trader_id":"M1","order_id":"order-1"})");

    assert(first.status_code == 202);
    assert(first.body.find(R"("sequence":1)") != std::string::npos);
    assert(second.body.find(R"("sequence":2)") != std::string::npos);
    assert(api.sequencer_state().body == R"({"last_sequence":2})");
}

void test_concurrent_sequence_is_unique_and_gapless() {
    ome::CommandSequencer sequencer;
    std::vector<std::uint64_t> sequences;
    std::mutex result_mutex;
    std::vector<std::thread> workers;

    for (int i = 0; i < 32; ++i) {
        workers.emplace_back([&] {
            ome::Command command;
            command.type = ome::CommandType::OpenSession;
            const auto accepted = sequencer.submit(std::move(command));
            std::scoped_lock lock(result_mutex);
            sequences.push_back(accepted.sequence);
        });
    }
    for (auto& worker : workers) worker.join();

    std::sort(sequences.begin(), sequences.end());
    assert(sequences.size() == 32);
    for (std::size_t i = 0; i < sequences.size(); ++i) {
        assert(sequences[i] == i + 1);
    }
}

}  // namespace

int main() {
    test_integer_validation();
    test_validation_and_api_sequence();
    test_concurrent_sequence_is_unique_and_gapless();
    std::cout << "All tests passed\n";
}
