cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# project settings
project(cpx
    VERSION     0.1.0
    LANGUAGES   C CXX
)

option(CPX_BUILD_TESTS      "Build unit tests"                            OFF)
option(CPX_BUILD_EXAMPLES   "Build unit tests"                            OFF)
option(CPX_REQUIREMENTS     "Only populate the external targets"          OFF)

message(STATUS "CPX_VERSION        : ${CPX_VERSION}")
message(STATUS "CPX_BUILD_TESTS    : ${CPX_BUILD_TESTS}")
message(STATUS "CPX_BUILD_EXAMPLES : ${CPX_BUILD_EXAMPLES}")
message(STATUS "CPX_REQUIREMENTS   : ${CPX_REQUIREMENTS}")


# core interface
add_library(cpx INTERFACE)
add_library(cpx::cpx ALIAS cpx)

target_include_directories(cpx INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if (NOT CPX_BUILD_TESTS AND NOT CPX_BUILD_EXAMPLES AND NOT CPX_REQUIREMENTS)
    return()
endif()


# system libraries
find_package(SQLite3 REQUIRED)
find_package(Protobuf REQUIRED STATIC)


# external libraries
include(cmake/CPM.cmake)

## String formatter using fmtlib
CPMAddPackage(
    NAME            fmt
    GIT_REPOSITORY  "https://github.com/fmtlib/fmt"
    GIT_TAG         11.2.0
    COMPONENTS      fmt
)

## Logging
CPMAddPackage(
    NAME            spdlog
    GIT_REPOSITORY  "https://github.com/gabime/spdlog"
    GIT_TAG         v1.15.3
    OPTIONS         "SPDLOG_FMT_EXTERNAL ON"
    COMPONENTS      spdlog
)

## JSON lib
CPMAddPackage(
    NAME            nlohmann_json
    URL             "https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz"
)

## Faster JSON lib
CPMAddPackage(
    NAME            yyjson
    GIT_REPOSITORY  "https://github.com/ibireme/yyjson"
    GIT_TAG         0.12.0
    OPTIONS         "YYJSON_INSTALL OFF"
)

## rapidjson
CPMAddPackage(
    NAME            rapidjson
    GIT_REPOSITORY  "https://github.com/Tencent/rapidjson"
    GIT_TAG         v1.1.0
    DOWNLOAD_ONLY   YES
)
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE "${rapidjson_SOURCE_DIR}/include")

## toml lib
CPMAddPackage(
    NAME            tomlplusplus
    GIT_REPOSITORY  "https://github.com/marzer/tomlplusplus"
    GIT_TAG         v3.4.0
)

CPMAddPackage(
    NAME            toml11
    GIT_REPOSITORY  "https://github.com/ToruNiina/toml11"
    GIT_TAG         v4.4.0
    OPTIONS         "TOML11_INSTALL OFF"
)

## Boost's struct reflection
CPMAddPackage(
    NAME            pfr
    GIT_REPOSITORY  "https://github.com/boostorg/pfr"
    GIT_TAG         boost-1.88.0
    OPTIONS         "BOOST_USE_MODULES OFF"
    COMPONENTS      Boost::pfr
)

## Enum reflection
CPMAddPackage(
    NAME            magic_enum
    GIT_REPOSITORY  "https://github.com/Neargye/magic_enum"
    GIT_TAG         v0.9.7
)

## Option parser
CPMAddPackage(
    NAME            cxxopts
    GIT_REPOSITORY  "https://github.com/jarro2783/cxxopts"
    GIT_TAG         v3.2.1
)

CPMAddPackage(
    NAME            CLI11
    GIT_REPOSITORY  "https://github.com/CLIUtils/CLI11"
    GIT_TAG         v2.6.1
)

## httplib
CPMAddPackage(
    NAME            httplib
    GIT_REPOSITORY  "https://github.com/yhirose/cpp-httplib"
    GIT_TAG         v0.41.0
    OPTIONS         "HTTPLIB_COMPILE ON" "HTTPLIB_USE_ZSTD_IF_AVAILABLE OFF" "HTTPLIB_REQUIRE_OPENSSL ON" "HTTPLIB_INSTALL OFF"
)

## Tests using GoogleTest
CPMAddPackage(
    NAME            googletest
    GIT_REPOSITORY  "https://github.com/google/googletest"
    GIT_TAG         v1.15.2
    OPTIONS         "INSTALL_GTEST OFF"
    COMPONENTS      gtest_main
)

CPMAddPackage(
    NAME            benchmark
    GIT_REPOSITORY  "https://github.com/google/benchmark"
    GIT_TAG         v1.9.2
    OPTIONS         "BENCHMARK_ENABLE_INSTALL OFF" "BENCHMARK_INSTALL_DOCS OFF" "BENCHMARK_INSTALL_TOOLS OFF" "BENCHMARK_USE_BUNDLED_GTEST OFF"
    COMPONENTS      benchmark_main
)

# only setup the exeternal libraries
if (CPX_REQUIREMENTS)
    return()
endif()


# private components
add_library(cpx_private INTERFACE)
add_library(cpx::private ALIAS cpx_private)

target_link_libraries(cpx_private INTERFACE
    fmt
    spdlog
    nlohmann_json
    yyjson
    rapidjson
    tomlplusplus::tomlplusplus
    toml11::toml11
    Boost::pfr
    magic_enum
    cxxopts
    CLI11
    protobuf::libprotobuf
)

target_compile_options(cpx_private INTERFACE
    -Wno-unused-parameter
    -Wno-missing-field-initializers
)

target_compile_options(cpx_private INTERFACE
    -Wall
    -Wextra
    -fmacro-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=cpp++
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-redundant-move>
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

target_link_options(cpx_private INTERFACE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)


# tests
if (CPX_BUILD_TESTS)
    # unit tests
    file(GLOB TEST_SOURCES tests/*)
    add_executable(test_all ${TEST_SOURCES})
    target_compile_features(test_all PRIVATE cxx_std_17)

    target_link_libraries(test_all PRIVATE
        cpx
        cpx_private
        gtest_main
        httplib
        SQLite::SQLite3
    )

	enable_testing()
    add_test(NAME test_all COMMAND test_all)

    # benchmark
    file(GLOB BENCH_SOURCES tests/bench/*.cpp)
    file(GLOB BENCH_PROTOS tests/bench/*.proto)
    add_executable(bench_all ${BENCH_SOURCES})

    target_compile_features(bench_all PRIVATE cxx_std_17)
    target_include_directories(bench_all PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

    protobuf_generate(
        TARGET   bench_all
        LANGUAGE cpp
        PROTOS   "${BENCH_PROTOS}"
    )

    target_link_libraries(bench_all PRIVATE
        cpx
        cpx_private
        benchmark_main
    )
endif()


# examples
if (CPX_BUILD_EXAMPLES)
    file(GLOB_RECURSE SRCS examples/0-basic/*)
    add_executable(example_basic ${SRCS})
    target_compile_features(example_basic PRIVATE cxx_std_17)

    target_link_libraries(example_basic PRIVATE
        cpx
        fmt
        yyjson
        tomlplusplus::tomlplusplus
        CLI11
    )

    file(GLOB_RECURSE SRCS examples/1-struct/*)
    add_executable(example_struct ${SRCS})
    target_compile_features(example_struct PRIVATE cxx_std_17)

    target_link_libraries(example_struct PRIVATE
        cpx
        fmt
        yyjson
        tomlplusplus::tomlplusplus
        Boost::pfr
    )

    file(GLOB_RECURSE SRCS examples/2-enum/*)
    add_executable(example_enum ${SRCS})
    target_compile_features(example_enum PRIVATE cxx_std_17)

    target_link_libraries(example_enum PRIVATE
        cpx
        fmt
        tomlplusplus::tomlplusplus
        magic_enum
    )

    file(GLOB_RECURSE SRCS examples/3-cli/*)
    add_executable(example_cli ${SRCS})
    target_compile_features(example_cli PRIVATE cxx_std_17)

    target_link_libraries(example_cli PRIVATE
        cpx
        fmt
        Boost::pfr
        CLI11
        magic_enum
    )

    file(GLOB_RECURSE SRCS examples/4-sql/*)
    add_executable(example_sql ${SRCS})
    target_compile_features(example_sql PRIVATE cxx_std_17)

    target_link_libraries(example_sql PRIVATE
        cpx
        fmt
        Boost::pfr
    )

    file(GLOB_RECURSE SRCS examples/5-protobuf/*)
    add_executable(example_protobuf ${SRCS})
    target_compile_features(example_protobuf PRIVATE cxx_std_17)

    target_link_libraries(example_protobuf PRIVATE
        cpx
        fmt
        Boost::pfr
        protobuf::libprotobuf
    )
endif()
