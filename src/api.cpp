#include "ome/api.hpp"

#include <crow/json.h>

#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ome {
namespace {

std::optional<std::string> json_string(const crow::json::rvalue& body, std::string_view key) {
    if (!body.has(key.data())) {
        return std::nullopt;
    }

    const auto& value = body[key.data()];
    if (value.t() != crow::json::type::String) {
        return std::nullopt;
    }
    return std::string(value.s());
}

std::optional<std::int64_t> json_integer(const crow::json::rvalue& body, std::string_view key) {
    if (!body.has(key.data())) {
        return std::nullopt;
    }

    const auto& value = body[key.data()];
    if (value.t() != crow::json::type::Number ||
        value.nt() == crow::json::num_type::Floating_point ||
        value.nt() == crow::json::num_type::Double_precision_floating_point) {
        return std::nullopt;
    }

    try {
        if (value.nt() == crow::json::num_type::Signed_integer) {
            return value.i();
        }
        if (value.nt() == crow::json::num_type::Unsigned_integer) {
            const auto unsigned_value = value.u();
            if (unsigned_value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                return std::nullopt;
            }
            return static_cast<std::int64_t>(unsigned_value);
        }
    } catch (const std::runtime_error&) {
        return std::nullopt;
    }
    return std::nullopt;
}

ApiResponse json_response(int status_code, crow::json::wvalue body) {
    return {status_code, body.dump()};
}

ApiResponse bad_request(std::string message) {
    crow::json::wvalue body;
    body["error"] = std::move(message);
    return json_response(400, std::move(body));
}

bool blank(const std::optional<std::string>& value) {
    return !value || value->empty();
}

}  // namespace

std::string to_string(CommandType type) {
    switch (type) {
        case CommandType::PlaceLimit: return "PLACE_LIMIT";
        case CommandType::CancelOrder: return "CANCEL_ORDER";
        case CommandType::OpenSession: return "OPEN_SESSION";
        case CommandType::CloseSession: return "CLOSE_SESSION";
    }
    return "UNKNOWN";
}

ApiResponse CommandApi::health() const {
    crow::json::wvalue body;
    body["status"] = "ok";
    body["component"] = "api";
    return json_response(200, std::move(body));
}

ApiResponse CommandApi::sequencer_state() const {
    crow::json::wvalue body;
    body["last_sequence"] = sequencer_.last_sequence();
    return json_response(200, std::move(body));
}

ApiResponse CommandApi::submit_command(const std::string& json_body) {
    const auto body = crow::json::load(json_body);
    if (!body || body.t() != crow::json::type::Object) {
        return bad_request("request body must be a JSON object");
    }

    const auto type = json_string(body, "type");
    if (blank(type)) {
        return bad_request("field 'type' is required");
    }

    Command command;
    if (*type == "PLACE_LIMIT") {
        command.type = CommandType::PlaceLimit;
        const auto trader_id = json_string(body, "trader_id");
        const auto side = json_string(body, "side");
        const auto price = json_integer(body, "price");
        const auto quantity = json_integer(body, "quantity");

        if (blank(trader_id)) return bad_request("field 'trader_id' is required");
        if (!side || (*side != "BUY" && *side != "SELL")) {
            return bad_request("field 'side' must be BUY or SELL");
        }
        if (!price || *price <= 0) return bad_request("field 'price' must be a positive integer");
        if (!quantity || *quantity <= 0) return bad_request("field 'quantity' must be a positive integer");

        command.trader_id = *trader_id;
        command.side = *side;
        command.price = price;
        command.quantity = quantity;
    } else if (*type == "CANCEL_ORDER") {
        command.type = CommandType::CancelOrder;
        const auto trader_id = json_string(body, "trader_id");
        const auto order_id = json_string(body, "order_id");
        if (blank(trader_id)) return bad_request("field 'trader_id' is required");
        if (blank(order_id)) return bad_request("field 'order_id' is required");
        command.trader_id = *trader_id;
        command.order_id = *order_id;
    } else if (*type == "OPEN_SESSION") {
        command.type = CommandType::OpenSession;
    } else if (*type == "CLOSE_SESSION") {
        command.type = CommandType::CloseSession;
    } else {
        return bad_request("unsupported command type");
    }

    const auto accepted = sequencer_.submit(std::move(command));
    crow::json::wvalue response;
    response["sequence"] = accepted.sequence;
    response["type"] = to_string(accepted.command.type);
    response["status"] = accepted.status;
    return json_response(202, std::move(response));
}

}  // namespace ome
