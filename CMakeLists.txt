cmake_minimum_required(VERSION 3.10)

# Name of Project 
project(GoTradeX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Add the source files required
set(SOURCES
    DeribitClient.cpp
    TradingSystem.cpp
    Rtm_Server.cpp
    main.cpp
)

# This will Create the executable
add_executable(d ${SOURCES})

# Link libraries (Boost, OpenSSL) as required
target_link_libraries(d
    boost_system 
    boost_json
    ssl
    crypto
)
