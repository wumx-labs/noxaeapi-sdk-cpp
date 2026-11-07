cmake_minimum_required(VERSION 3.14)
project(noxaeapi-sdk-cpp VERSION 0.7.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(noxaeapi-sdk INTERFACE)
add_library(noxaeapi::sdk ALIAS noxaeapi-sdk)

target_include_directories(noxaeapi-sdk INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

find_package(CURL)
if (CURL_FOUND)
    target_link_libraries(noxaeapi-sdk INTERFACE CURL::libcurl)
    target_compile_definitions(noxaeapi-sdk INTERFACE NOXAEAPI_USE_CURL=1)
endif()
