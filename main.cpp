#include "TradingSystem.hpp"  // Include TradingSystem class header

#include <iostream>  // Standard I/O stream for error messages

int main() {
    const char* client_id = std::getenv("DERIBIT_CLIENT_ID");  // Retrieve client ID from environment variable
    const char* client_secret = std::getenv("DERIBIT_CLIENT_SECRET");  // Retrieve client secret from environment variable
    if (!client_id || !client_secret) {  // Check if environment variables are set
        std::cerr << "Environment variables DERIBIT_CLIENT_ID and DERIBIT_CLIENT_SECRET must be set\n";  // Error if variables are missing
        return 1;
    }

    try {
        TradingSystem system("test.deribit.com", "443", client_id, client_secret);  // Initialize TradingSystem with connection details
        system.main_menu();  // Display main menu for user interaction
    } catch (const std::exception& e) {  // Catch any exceptions and display error
        std::cerr << "Fatal error: " << e.what() << "\n";  // Print exception message
        return 1;
    }

    return 0;  // Return success
}
