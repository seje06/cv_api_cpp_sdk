cmake_minimum_required(VERSION 3.16)
project(ImageApiSdk VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# vcpkg를 사용할 때는 CONFIG 모드로 찾는 편이 더 안정적이다.
find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(ImageApiSdk STATIC
    src/ImageApiClient.cpp
    src/HttpUtils.cpp
    src/Operations.cpp
)

# 공개 헤더는 라이브러리를 사용하는 쪽에서도 보여야 한다.
target_include_directories(ImageApiSdk
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ImageApiSdk
    PUBLIC
        CURL::libcurl
        nlohmann_json::nlohmann_json
)

add_executable(ImageApiSdkExample
    example/main.cpp
)

target_link_libraries(ImageApiSdkExample PRIVATE ImageApiSdk)