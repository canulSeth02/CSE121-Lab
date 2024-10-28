cmake_minimum_required(VERSION 3.5)

# Define the project name
project(MyESP32C3Project)

# Include the ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Add executable for the main application
add_executable(${PROJECT_NAME} main.cpp DFRobot_RGBLCD1602.cpp)

# Link the necessary libraries
target_link_libraries(${PROJECT_NAME} PRIVATE idf::espidf)

# Set the compilation standard for C++ (e.g., C++17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

# Include directories for header files
target_include_directories(${PROJECT_NAME} PRIVATE .)

# Run the ESP-IDF project function
project(${PROJECT_NAME})
