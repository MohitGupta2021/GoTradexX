#ifndef RTM_SERVER_HPP
#define RTM_SERVER_HPP

#include <boost/asio.hpp>  // Basic networking and asynchronous I/O
#include <boost/beast.hpp>  // HTTP and WebSocket handling
#include <boost/beast/core.hpp>  // Core Beast components for message parsing
#include <boost/beast/websocket.hpp>  // WebSocket connection management
#include <boost/beast/ssl.hpp>  // SSL/TLS support for secure connections
#include <boost/asio/ssl/stream.hpp>  // SSL stream wrapper for sockets
#include <boost/asio/connect.hpp>  // Socket connection helpers
#include <boost/json.hpp>  // JSON parsing and serialization
#include <iostream>  // Standard input/output streams
#include <string>  // String manipulation
#include <unordered_map>  // Fast key-value lookups
#include <unordered_set>  // Fast unique item storage
#include <mutex>  // Mutex for thread safety
#include <condition_variable>  // Thread synchronization
#include <thread>  // Thread management


namespace beast = boost::beast;  // Alias for Boost.Beast library
namespace websocket = beast::websocket;  // Alias for WebSocket functionalities in Beast
namespace net = boost::asio;  // Alias for Boost.Asio library
namespace ssl = boost::asio::ssl;  // Alias for Boost.Asio SSL functionality
namespace json = boost::json;  // Alias for Boost.JSON library
using tcp = net::ip::tcp;  // Alias for TCP socket type in Boost.Asio

class Rtm_Server {
public:
  Rtm_Server();  // Constructor for WebSocket class
    void connect();  // Establish WebSocket connection
    void stream_orderbook_updates();  // Stream real-time orderbook data
    void run();  // Start WebSocket communication loop

private:
    void on_message(const std::string &message);  // Callback for handling incoming messages

    net::io_context ioc;  // I/O context for async operations
    ssl::context ctx;  // SSL context for secure WebSocket connections
    websocket::stream<beast::ssl_stream<tcp::socket>> ws;  // Secure WebSocket stream
    std::mutex data_mutex;  // Mutex for protecting shared data
    std::condition_variable data_cv;  // Condition variable for thread synchronization
    std::unordered_map<std::string, json::value> orderbook_data;  // Store orderbook data
};

#endif