#include "TradingSystem.hpp"
#include <iostream>
#include <limits>
#include <chrono>
#include <iomanip>
#include "Rtm_Server.hpp"

// Constructor to initialize the TradingSystem with client connection details.
TradingSystem::TradingSystem(const std::string& host, const std::string& port, const char* client_id, const char* client_secret)
    : client(host, port, client_id, client_secret), state_for_full_result(false) {}

// Handles responses when multiple positions are returned.
void TradingSystem::handle_response_all(const json::value& response) {
    if (response.as_object().contains("result")) {
        const auto& result = response.at("result");

        if (result.is_array()) {  // Handling multiple positions
            std::cout << "\n========== Positions ==========\n";
            for (const auto& item : result.as_array()) {
                std::cout << "----------------------------------\n";
                for (const auto& [key, value] : item.as_object()) {
                    std::cout << std::setw(20) << std::left << key << ": " << json::serialize(value) << "\n";
                }
            }
            std::cout << "================================\n";
        } else if (result.is_object()) {  // Handling a single object response
            std::cout << "\n========== Response ==========\n";
            for (const auto& [key, value] : result.as_object()) {
                std::cout << std::setw(20) << std::left << key << ": " << json::serialize(value) << "\n";
            }
            std::cout << "================================\n";
        }
    } else if (response.as_object().contains("error")) {  // Error handling
        std::cerr << "\n[ERROR] " << response.at("error").at("message").as_string() << "\n";
    } else {
        std::cerr << "\n[WARNING] Unexpected response format.\n";
    }
}

// Handles responses when a single order or bid-related response is returned.
void TradingSystem::handle_response(const json::value& response) {
    if (response.as_object().contains("result")) {
        const auto& result = response.at("result").as_object();

        if (result.contains("order")) {  // Handling order result
            std::cout << "Order ID: " << result.at("order").at("order_id").as_string() << "\n";
        } 
        else if (result.contains("bids") && result.at("bids").is_array()) {  // Handling order book bids
            std::cout << "Order Book:\n";
            for (const auto& bid : result.at("bids").as_array()) {
                std::cout << "Bid: " << bid.at(0).as_double() << " @ " << bid.at(1).as_double() << "\n";
            }
        }
    } else if (response.as_object().contains("error")) {  // Error handling
        std::cerr << "Error: " << response.at("error").at("message").as_string() << "\n";
    }
}

// Function that checks if the result should be fully displayed (for debugging).
bool TradingSystem::check_full_result() {
    char check = 'a';
    return check == 'a';
}

// Displays the main menu options for trading system operations.
void display_menu() {
    std::cout << "\033[1;36m\n==============================\n";
    std::cout << "WELCOME TO GOTRADEX TRADING SYSTEM\n";
    std::cout << "==============================\033[0m\n";
    
    std::cout << "\033[1;33m1.  Buy Order\n";
    std::cout << "2.  Sell Order\n";
    std::cout << "3.  Cancel Order\n";
    std::cout << "4.  Modify Order\n";
    std::cout << "5.  Get Orderbook\n";
    std::cout << "6.  View Positions\n";
    std::cout << "7.  Get Market Price\n";
    std::cout << "8.  Get Real-Time Data\n";
    std::cout << "9.  Exit\n\033[0m";

    std::cout << "\nEnter your choice: ";
}

// Measures execution time for various operations.
void TradingSystem::measure_execution_time(std::function<void()> func, const std::string& operation_name) {
    auto start_time = std::chrono::high_resolution_clock::now();
    func();  // Execute the function
    auto end_time = std::chrono::high_resolution_clock::now();
    
    double elapsed_time = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    std::cout << "\033[1;35m⏳ Execution time for " << operation_name << ": " 
              << elapsed_time << " ms\033[0m\n";
}

// Main function to handle the trading system logic and display the menu.
void TradingSystem::main_menu() {
    try {
        measure_execution_time([this]() {
            client.connect();  // Connect to the trading client
            client.authenticate();  // Authenticate the client
        }, "Initialization");

    } catch (const std::exception& e) {
        std::cerr << "Initialization failed: " << e.what() << "\n";
        return;
    }

    while (true) {
        display_menu();  // Display the menu to the user
        int choice;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Clean up input buffer

        bool state_for_full_result = false;
        char check = 'a';  // This check could be used to toggle full results
        if (check == 'a') {
            state_for_full_result = true;
        }

        try {
            if (choice == 1 || choice == 2) {  // Handling Buy and Sell Orders
                std::string instrument, type;
                int quantity;
                double price;

                std::cout << "Instrument: ";
                std::getline(std::cin, instrument);
                std::cout << "Type (limit/market): ";
                std::getline(std::cin, type);
                std::cout << "Quantity: ";
                std::cin >> quantity;
                std::cout << "Price: ";
                std::cin >> price;
                std::cin.ignore();

                // Measure execution time for placing the order
                measure_execution_time([this, &instrument, &type, quantity, price, choice, state_for_full_result]() {
                    auto response = (choice == 1) ? client.place_order(instrument, type, quantity, price) 
                                                  : client.sell_order(instrument, type, quantity, price);

                    if (state_for_full_result) {
                        handle_response_all(response);  // Handle full response
                    } else {
                        handle_response(response);  // Handle normal response
                    }
                }, (choice == 1) ? "Buy Order" : "Sell Order");

            } else if (choice == 3) {  // Cancel Order
                std::string order_id;
                std::cout << "Order ID: ";
                std::getline(std::cin, order_id);

                // Measure execution time for canceling the order
                measure_execution_time([this, &order_id, state_for_full_result]() {
                    if (state_for_full_result) {
                        handle_response_all(client.cancel_order(order_id));  // Full response handling
                    } else {
                        handle_response(client.cancel_order(order_id));  // Normal response handling
                    }
                }, "Cancel Order");

            } else if (choice == 4) {  // Modify Order
                std::string order_id;
                double new_price, amount;
                std::cout << "Order ID: ";
                std::getline(std::cin, order_id);
                std::cout << "Amount: ";
                std::cin >> amount;
                std::cout << "New Price: ";
                std::cin >> new_price;
                std::cin.ignore();

                // Measure execution time for modifying the order
                measure_execution_time([this, &order_id, amount, new_price, state_for_full_result]() {
                    if (state_for_full_result) {
                        handle_response_all(client.modify_order(order_id, amount, new_price));
                    } else {
                        handle_response(client.modify_order(order_id, amount, new_price));
                    }
                }, "Modify Order");

            } else if (choice == 5) {  // Get Orderbook
                std::string instrument;
                std::cout << "Instrument: ";
                std::getline(std::cin, instrument);

                // Measure execution time for fetching the orderbook
                measure_execution_time([this, &instrument, state_for_full_result]() {
                    if (state_for_full_result) {
                        handle_response_all(client.get_orderbook(instrument));
                    } else {
                        handle_response(client.get_orderbook(instrument));
                    }
                }, "Get Orderbook");

            } else if (choice == 6) {  // View Positions
                std::string currency, kind;
                std::cout << "Currency (BTC/ETH): ";
                std::getline(std::cin, currency);
                std::cout << "Kind (future/market): ";
                std::getline(std::cin, kind);

                if (currency != "BTC" && currency != "ETH") {
                    std::cerr << "Error: Invalid currency. Please enter 'BTC' or 'ETH'.\n";
                    continue;
                }
                if (kind != "future" && kind != "market") {
                    std::cerr << "Error: Invalid kind. Please enter 'future' or 'market'.\n";
                    continue;
                }

                // Measure execution time for viewing positions
                measure_execution_time([this, &currency, &kind, state_for_full_result]() {
                    if (state_for_full_result) {
                        handle_response_all(client.view_positions(currency, kind));
                    } else {
                        handle_response(client.view_positions(currency, kind));
                    }
                }, "View Positions");

            } else if (choice == 7) {  // Get Market Price
                std::string instrument_name;
                std::cout << "Instrument Name: ";
                std::getline(std::cin, instrument_name);

                // Measure execution time for fetching market price
                measure_execution_time([this, &instrument_name, state_for_full_result]() {
                    if (state_for_full_result) {
                        handle_response_all(client.get_market_price_by_instruments(instrument_name));
                    } else {
                        handle_response(client.get_market_price_by_instruments(instrument_name));
                    }
                }, "Get Market Price");

            } else if (choice == 8) {  // Real-Time Data
                Rtm_Server websocket;
                websocket.run();
            }
            else if (choice == 9) {  // Exit
                break;
            }
        } catch (const std::exception& e) {  // Error handling for operations
            std::cerr << "Operation failed: " << e.what() << "\n";
        }
    }
}
