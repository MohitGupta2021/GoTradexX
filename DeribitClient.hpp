
#ifndef DERIBIT_CLIENT_HPP
#define DERIBIT_CLIENT_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/connect.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <string>
#include <atomic>

namespace beast = boost::beast;  // Alias for Boost.Beast
namespace websocket = beast::websocket;  // Alias for WebSocket in Beast
namespace net = boost::asio;  // Alias for Boost.Asio
namespace json = boost::json;  // Alias for Boost.JSON
using tcp = net::ip::tcp;  // Alias for TCP socket type

class DeribitClient {
public:
    DeribitClient(const std::string& host, const std::string& port, const std::string& client_id, const std::string& client_secret);  // Constructor to initialize with connection details
    ~DeribitClient();  // Destructor
    void connect();  // Establish WebSocket connection to Deribit
    void authenticate();  // Authenticate client with Deribit
    json::value place_order(const std::string& instrument, const std::string& type, int quantity, double price);  // Place a new order
    json::value sell_order(const std::string& instrument, const std::string& type, int quantity, double price);  // Place a sell order
    json::value get_market_price_by_instruments(const std::string& instrument_name);  // Get market price for a given instrument
    json::value cancel_order(const std::string& order_id);  // Cancel an existing order
    json::value modify_order(const std::string& order_id, double amount, double new_price);  // Modify an existing order
    json::value get_orderbook(const std::string& instrument);  // Retrieve orderbook data for an instrument
    json::value view_positions(const std::string& currency, const std::string& kind);  // View current positions in a specific currency

private:
    json::value send_request(const json::value& payload);  // Send a JSON request and return the response
    net::io_context ioc_;  // I/O context for asynchronous operations
    net::ssl::context ctx_{net::ssl::context::tlsv12_client};  // SSL context for secure communication
    tcp::resolver resolver_;  // Resolver for looking up host addresses
    websocket::stream<beast::ssl_stream<tcp::socket>> ws_;  // WebSocket stream wrapped with SSL
    std::string host_, port_, client_id_, client_secret_;  // Connection details
    std::atomic<int> current_id_{0};  // Atomic counter for tracking request IDs
};


#endif