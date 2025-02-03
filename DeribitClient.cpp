
#include "DeribitClient.hpp"

// Constructor: Initializes WebSocket and SSL context
DeribitClient::DeribitClient(const std::string& host, const std::string& port, const std::string& client_id, const std::string& client_secret)
        : resolver_(ioc_), ws_(ioc_, ctx_), host_(host), port_(port), client_id_(client_id), client_secret_(client_secret) {
        ctx_.set_default_verify_paths();
        ctx_.set_verify_mode(net::ssl::verify_peer);
    }

// Destructor: Closes WebSocket connection
DeribitClient:: ~DeribitClient() {
        try {
            ws_.close(websocket::close_code::normal);
        } catch (const std::exception& e) {
            std::cerr << "WebSocket close error: " << e.what() << "\n";
        }
    }

// Connect to Deribit WebSocket API
void DeribitClient:: connect() {
        auto const results = resolver_.resolve(host_, port_);
        net::connect(beast::get_lowest_layer(ws_), results.begin(), results.end());
        ws_.next_layer().handshake(net::ssl::stream_base::client);
        ws_.binary(false);
        ws_.handshake(host_, "/ws/api/v2");
    }

// Authenticate with API using client credentials
void DeribitClient:: authenticate() {
        json::value auth_payload = {
            {"jsonrpc", "2.0"},
            {"id", ++current_id_},
            {"method", "public/auth"},
            {"params", {
                {"grant_type", "client_credentials"},
                {"client_id", client_id_},
                {"client_secret", client_secret_}
            }}
        };
        send_request(auth_payload);
    }

// Send request via WebSocket and return response
json::value DeribitClient:: send_request(const json::value& payload) {
        ws_.write(net::buffer(json::serialize(payload)));
        beast::flat_buffer buffer;
        ws_.read(buffer);
        std::string response = beast::buffers_to_string(buffer.data());
        return json::parse(response);
    }

// Place a buy order
json::value DeribitClient:: place_order(const std::string& instrument, const std::string& type, int quantity, double price) {
        json::value order_payload = {
            {"jsonrpc", "2.0"},
            {"id", ++current_id_},
            {"method", "private/buy"},
            {"params", {
                {"instrument_name", instrument},
                {"amount", quantity},
                {"type", type},
                {"price", price}
            }}
        };
        return send_request(order_payload);
    }

// Place a sell order
json::value DeribitClient:: sell_order(const std::string& instrument, const std::string& type, int quantity, double price) {
        json::value order_payload = {
            {"jsonrpc", "2.0"},
            {"id", ++current_id_},
            {"method", "private/sell"},
            {"params", {
                {"instrument_name", instrument},
                {"amount", quantity},
                {"type", type},
                {"price", price}
            }}
        };
        return send_request(order_payload);
    }

// Get last market price for an instrument
json::value DeribitClient:: get_market_price_by_instruments(const std::string& instrument_name) {
        json::value cancel_payload = {
            {"jsonrpc", "2.0"},
            {"id", ++current_id_},
            {"method", "public/get_last_trades_by_instrument"},
            {"params", { {"instrument_name", instrument_name},{"count", 1} }}
        };
        return send_request(cancel_payload);
    }

// Cancel an order
json::value DeribitClient:: cancel_order(const std::string& order_id) {
        json::value cancel_payload = {
            {"jsonrpc", "2.0"},
            {"id", ++current_id_},
            {"method", "private/cancel"},
            {"params", { {"order_id", order_id} }}
        };
        return send_request(cancel_payload);
    }

// Modify an existing order
json::value DeribitClient:: modify_order(const std::string& order_id, double amount, double new_price) {
        json::value modify_payload = {
            {"jsonrpc", "2.0"},
            {"id", ++current_id_},
            {"method", "private/edit"},
            {"params", { {"order_id", order_id}, {"amount", amount}, {"price", new_price} }}
        };
        return send_request(modify_payload);
    }

// Get the order book for an instrument
json::value DeribitClient:: get_orderbook(const std::string& instrument) {
        json::value orderbook_payload = {
            {"jsonrpc", "2.0"},
            {"id", ++current_id_},
            {"method", "public/get_order_book"},
            {"params", { {"instrument_name", instrument} , {"depth" , 5}}}
        };
        return send_request(orderbook_payload);
    }

// View open positions
json::value DeribitClient:: view_positions(const std::string& currency, const std::string& kind) {
        json::value positions_payload = {
            {"jsonrpc", "2.0"},
            {"id", ++current_id_},
            {"method", "private/get_positions"},
            {"params", { {"currency", currency}, {"kind", kind} }}
        };
        return send_request(positions_payload);
    }
