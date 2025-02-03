


#include "Rtm_Server.hpp"

// Constants for WebSocket connection
const std::string HOST = "test.deribit.com";
const std::string PORT = "443";
const std::string TARGET = "/ws/api/v2";

// Constructor to initialize WebSocket context and connection
Rtm_Server::Rtm_Server() 
    : ctx(ssl::context::tlsv12_client), ws(ioc, ctx) {}

// Callback function to process incoming messages from WebSocket
void Rtm_Server::on_message(const std::string &message) {
    std::lock_guard<std::mutex> lock(data_mutex);  // Thread-safe access to shared data
    json::value msg_json = json::parse(message);

    // Check if message contains "params" and process channel data
    if (msg_json.as_object().contains("params")) {
        auto &params = msg_json.at("params").as_object();
        if (params.contains("channel")) {
            std::string channel = params["channel"].as_string().c_str();
            orderbook_data[channel] = params["data"];
            data_cv.notify_all();  // Notify waiting thread for new data
        }
    }
}

// Function to establish WebSocket connection and subscribe to channels
void Rtm_Server::connect() {
    try {
        tcp::resolver resolver(ioc);
        auto const results = resolver.resolve(HOST, PORT);
        net::connect(beast::get_lowest_layer(ws), results.begin(), results.end());

        ws.next_layer().handshake(ssl::stream_base::client);  // Perform SSL handshake
        ws.handshake(HOST, TARGET);  // Perform WebSocket handshake

        std::cout << "Connected to Deribit test WebSocket!" << std::endl;

        // Subscribe to orderbook update channels
        json::value subscription_message = {
            {"jsonrpc", "2.0"},
            {"id", 42},
            {"method", "public/subscribe"},
            {"params", {
                {"channels", json::array{
                    "deribit_price_index.btc_usd",
                    "deribit_price_index.algo_usd",
                    "deribit_price_index.bch_usd"
                }}
            }}
        };

        ws.write(net::buffer(json::serialize(subscription_message)));

        // Read incoming messages and process them
        while (true) {
            beast::flat_buffer buffer;
            ws.read(buffer);
            std::string message = beast::buffers_to_string(buffer.data());
            on_message(message);  // Handle the incoming message
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;  // Handle exceptions
    }
}

// Function to stream orderbook updates
void Rtm_Server::stream_orderbook_updates() {
    while (true) {
        std::unique_lock<std::mutex> lock(data_mutex);  // Lock for safe data access
        data_cv.wait(lock, [this] { return !orderbook_data.empty(); });  // Wait for data

        // Print out orderbook data from each channel
        for (const auto &[channel, data] : orderbook_data) {
            std::cout << "Channel: " << channel << std::endl;
            std::cout << "Data: " << json::serialize(data) << std::endl;
        }
        orderbook_data.clear();  // Clear data after processing
    }
}

// Function to run WebSocket connection and data streaming in separate threads
void Rtm_Server::run() {
    std::thread ws_thread(&Rtm_Server::connect, this);  // Thread for WebSocket connection
    std::thread stream_thread(&Rtm_Server::stream_orderbook_updates, this);  // Thread for data streaming

    ws_thread.join();  // Wait for WebSocket thread to finish
    stream_thread.join();  // Wait for streaming thread to finish
}
