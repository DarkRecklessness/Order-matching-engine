cmake_minimum_required(VERSION 3.20)

project(order_matching_engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-38-2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(asio)
set(ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/include" CACHE PATH "Asio include directory" FORCE)

set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CROW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CROW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.3.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Crow)

add_library(ome_core
    src/api.cpp
    src/sequencer.cpp
)
target_include_directories(ome_core PUBLIC include)
target_link_libraries(ome_core PUBLIC Crow::Crow)

add_executable(ome_server src/main.cpp)
target_link_libraries(ome_server PRIVATE ome_core)
if(WIN32)
    target_link_libraries(ome_server PRIVATE ws2_32 mswsock)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(ome_tests tests/sequencer_tests.cpp)
    target_link_libraries(ome_tests PRIVATE ome_core)
    add_test(NAME ome_tests COMMAND ome_tests)
endif()