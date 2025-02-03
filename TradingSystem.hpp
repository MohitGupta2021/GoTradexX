#ifndef TRADING_SYSTEM_HPP
#define TRADING_SYSTEM_HPP

#include "DeribitClient.hpp"  // Include custom Deribit client header for interaction with Deribit API

#include <functional>  // For using std::function to pass functions as arguments

class TradingSystem {
private:
    DeribitClient client;  // DeribitClient instance for interacting with the Deribit API
    bool state_for_full_result;  // State flag to track full result status
public:
    TradingSystem(const std::string& host, const std::string& port, const char* client_id, const char* client_secret);  // Constructor to initialize client with connection details
    void main_menu();  // Display main menu for trading system
    void handle_response_all(const json::value& response);  // Handle full response from API
    void handle_response(const json::value& response);  // Handle specific response from API
    bool check_full_result();  // Check if the full result is available

    void measure_execution_time(std::function<void()> func, const std::string& operation_name);  // Measure execution time of a function using std::function
};


#endif 
