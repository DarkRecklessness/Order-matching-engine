#include "ome/api.hpp"

#include <crow.h>

#include <charconv>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <utility>

namespace {

crow::response to_http_response(ome::ApiResponse api_response) {
    crow::response response(api_response.status_code, std::move(api_response.body));
    response.set_header("Content-Type", "application/json");
    return response;
}

}  // namespace

int main(int argc, char** argv) {
    std::uint16_t port = 8080;
    if (argc == 3 && std::string_view(argv[1]) == "--port") {
        unsigned int parsed{};
        const std::string_view value(argv[2]);
        const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), parsed);
        if (error != std::errc{} || end != value.data() + value.size() || parsed == 0 || parsed > 65535) {
            std::cerr << "Usage: ome_server [--port 1..65535]\n";
            return 2;
        }
        port = static_cast<std::uint16_t>(parsed);
    } else if (argc != 1) {
        std::cerr << "Usage: ome_server [--port 1..65535]\n";
        return 2;
    }

    ome::CommandApi api;
    crow::SimpleApp app;

    CROW_ROUTE(app, "/health")
        .methods(crow::HTTPMethod::GET)([&api] {
            return to_http_response(api.health());
        });

    CROW_ROUTE(app, "/v1/sequencer")
        .methods(crow::HTTPMethod::GET)([&api] {
            return to_http_response(api.sequencer_state());
        });

    CROW_ROUTE(app, "/v1/commands")
        .methods(crow::HTTPMethod::POST)([&api](const crow::request& request) {
            return to_http_response(api.submit_command(request.body));
        });

    std::cout << "OME API listening on http://127.0.0.1:" << port << '\n';
    app.bindaddr("127.0.0.1").port(port).multithreaded().run();
}