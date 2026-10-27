cmake_minimum_required(VERSION 3.21)
project(RaylibGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Automatically finds the global Homebrew installation
find_package(raylib CONFIG REQUIRED)

file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Injects Raylib and all required macOS frameworks automatically
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)