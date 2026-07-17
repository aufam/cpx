cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

# project settings
project(cpx
    VERSION     0.1.0
    LANGUAGES   C CXX
)

option(CPX_BUILD_TESTS      "Build unit tests"                            OFF)
option(CPX_BUILD_BENCHMARK  "Build benchmark"                             OFF)
option(CPX_BUILD_EXAMPLES   "Build examples"                              OFF)
option(CPX_REQUIREMENTS     "Only populate the external targets"          OFF)
option(CPX_MODULES          "Enable module target"                        OFF)

message(STATUS "CPX_VERSION        : ${cpx_VERSION}")
message(STATUS "CPX_BUILD_TESTS    : ${CPX_BUILD_TESTS}")
message(STATUS "CPX_BUILD_BENCHMARK: ${CPX_BUILD_BENCHMARK}")
message(STATUS "CPX_BUILD_EXAMPLES : ${CPX_BUILD_EXAMPLES}")
message(STATUS "CPX_REQUIREMENTS   : ${CPX_REQUIREMENTS}")
message(STATUS "CPX_MODULES        : ${CPX_MODULES}")


# core interface
add_library(cpx INTERFACE)
add_library(cpx::cpx ALIAS cpx)

target_include_directories(cpx INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if (CPX_MODULES)
    add_library(cpx_mod)
    add_library(cpx::mod ALIAS cpx_mod)
    target_compile_features(cpx_mod PUBLIC cxx_std_20)

    file(GLOB MOD_SOURCES src/*.cppm)
    target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES ${MOD_SOURCES})
    target_link_libraries(cpx_mod PUBLIC cpx)
endif()

function(cpx_configure_modules)
    if (CPX_MODULES)
        if (TARGET Boost::pfr)
            target_link_libraries(cpx_mod PUBLIC Boost::pfr)
        endif()

        if (TARGET magic_enum)
            target_link_libraries(cpx_mod PUBLIC magic_enum)
        endif()

        if (TARGET fmt)
            target_link_libraries(cpx_mod PUBLIC fmt)
            target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                src/third_parties/fmt.cppm
            )
        endif()

        if (TARGET CLI11)
            target_link_libraries(cpx_mod PUBLIC CLI11)
            target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                src/third_parties/cli11.cppm
            )
        endif()

        if (TARGET yyjson)
            target_link_libraries(cpx_mod PUBLIC yyjson)
            target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                src/third_parties/yy_json.cppm
            )
        endif()

        if (TARGET nlohmann_json)
            target_link_libraries(cpx_mod PUBLIC nlohmann_json)
            target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                src/third_parties/nlohmann_json.cppm
            )
        endif()

        if (TARGET rapidjson)
            target_link_libraries(cpx_mod PUBLIC rapidjson)
            target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                src/third_parties/rapid_json.cppm
            )
        endif()

        if (TARGET toml11)
            target_link_libraries(cpx_mod PUBLIC toml11)
            target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                src/third_parties/toruniina_toml.cppm
            )
        endif()

        if (TARGET tomlplusplus)
            target_link_libraries(cpx_mod PUBLIC tomlplusplus)
            target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                src/third_parties/marzer_toml.cppm
            )
        endif()

        if (TARGET yaml-cpp)
            target_link_libraries(cpx_mod PUBLIC yaml-cpp)
            target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                src/third_parties/jbeder_yaml.cppm
            )
        endif()

        if (TARGET msgpack-cxx)
            target_link_libraries(cpx_mod PUBLIC msgpack-cxx)
            target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                src/third_parties/msgpack.cppm
            )
        endif()

        if (Protobuf_FOUND)
            target_link_libraries(cpx_mod PUBLIC protobuf::libprotobuf)
            target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                src/third_parties/protobuf.cppm
            )
        endif()

        if (SQLite3_FOUND)
            target_link_libraries(cpx_mod PUBLIC SQLite::SQLite3)
            target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                src/third_parties/sqlite.cppm
            )
        endif()

        if (PostgreSQL_FOUND)
            target_link_libraries(cpx_mod PUBLIC PostgreSQL::PostgreSQL)
            target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                src/third_parties/postgres.cppm
            )
        endif()

        if (MySQL_FOUND AND NOT MariaDB_FOUND)
            target_link_libraries(cpx_mod PUBLIC MySQL::mysql)
            target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                src/third_parties/mysql.cppm
            )
        endif()

        if (MariaDB_FOUND)
            target_link_libraries(cpx_mod PUBLIC MariaDB::mariadb)
            target_sources(cpx_mod PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                src/third_parties/mysql.cppm
            )
        endif()
    endif()
endfunction()

if (NOT CPX_BUILD_TESTS AND NOT CPX_BUILD_EXAMPLES AND NOT CPX_REQUIREMENTS)
    cpx_configure_modules()
    return()
endif()


# system libraries
find_package(SQLite3)
if (SQLite3_FOUND)
    message(STATUS "SQLite3 found.")
endif()

find_package(PostgreSQL)
if (PostgreSQL_FOUND)
    message(STATUS "PostgreSQL found.")
endif()

find_package(MySQL)
if (MySQL_FOUND)
    message(STATUS "MySQL found.")
endif()

find_package(MariaDB)
if (MariaDB_FOUND)
    message(STATUS "MariaDB found.")
endif()

find_package(Protobuf)
if (Protobuf_FOUND)
    message(STATUS "Protobuf found.")
endif()

find_package(absl)
if (absl_FOUND)
    message(STATUS "absl found.")
endif()


# external libraries
include(cmake/CPM.cmake)

## String formatter using fmtlib
CPMAddPackage(
    NAME            fmt
    GIT_REPOSITORY  "https://github.com/fmtlib/fmt"
    GIT_TAG         12.1.0
)

## Logging
CPMAddPackage(
    NAME            spdlog
    GIT_REPOSITORY  "https://github.com/gabime/spdlog"
    GIT_TAG         v1.15.3
    OPTIONS         "SPDLOG_FMT_EXTERNAL ON"
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
    GIT_TAG         master
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
    OPTIONS         "TOML11_INSTALL OFF" "TOML11_PRECOMPILE ON"
)
target_compile_options(toml11 PUBLIC "-Wno-deprecated-literal-operator")

## yaml
CPMAddPackage(
    NAME            yaml-cpp
    GIT_REPOSITORY  "https://github.com/jbeder/yaml-cpp"
    GIT_TAG         yaml-cpp-0.9.0
    OPTIONS         "YAML_CPP_BUILD_CONTRIB OFF"
                    "YAML_CPP_BUILD_TOOLS OFF"
)

## Boost's struct reflection
CPMAddPackage(
    NAME            pfr
    GIT_REPOSITORY  "https://github.com/boostorg/pfr"
    GIT_TAG         boost-1.88.0
    OPTIONS         "BOOST_USE_MODULES OFF"
)

## Enum reflection
CPMAddPackage(
    NAME            magic_enum
    GIT_REPOSITORY  "https://github.com/Neargye/magic_enum"
    GIT_TAG         v0.9.7
)

## Option parser
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
target_compile_options(httplib PRIVATE -Wno-deprecated-declarations)

## msgpack
CPMAddPackage(
    NAME            msgpack
    GIT_REPOSITORY  "https://github.com/msgpack/msgpack-c"
    GIT_TAG         cpp-7.0.0
    OPTIONS         "MSGPACK_USE_BOOST OFF"
)

## Tests using GoogleTest
CPMAddPackage(
    NAME            googletest
    GIT_REPOSITORY  "https://github.com/google/googletest"
    GIT_TAG         v1.15.2
    OPTIONS         "INSTALL_GTEST OFF"
)

CPMAddPackage(
    NAME            benchmark
    GIT_REPOSITORY  "https://github.com/google/benchmark"
    GIT_TAG         v1.9.2
    OPTIONS         "BENCHMARK_ENABLE_INSTALL OFF"
                    "BENCHMARK_INSTALL_DOCS OFF"
                    "BENCHMARK_INSTALL_TOOLS OFF"
                    "BENCHMARK_USE_BUNDLED_GTEST OFF"
                    "BENCHMARK_ENABLE_TESTING OFF"
)
target_compile_options(benchmark PRIVATE -Wno-c2y-extensions)
target_compile_options(benchmark_main PRIVATE -Wno-c2y-extensions)

# only setup the exeternal libraries
if (CPX_REQUIREMENTS)
    return()
endif()

# configure module third parties
cpx_configure_modules()

if (NOT Protobuf_FOUND)
    message(FATAL_ERROR "Protobuf not found.")
endif()

if (NOT SQLite3_FOUND)
    message(FATAL_ERROR "SQLite3 not found.")
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
    yaml-cpp
    Boost::pfr
    magic_enum
    CLI11
    protobuf::libprotobuf
    msgpack-cxx
)

if (TARGET absl::log)
    target_link_libraries(cpx_private INTERFACE absl::log)
endif()
if (TARGET absl::check)
    target_link_libraries(cpx_private INTERFACE absl::check)
endif()

target_compile_options(cpx_private INTERFACE
    -Wno-unused-parameter
    -Wno-missing-field-initializers
)

target_compile_options(cpx_private INTERFACE
    -Wall
    -Wextra
    -fmacro-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=cpx
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
endif()

# benchmark
if (CPX_BUILD_BENCHMARK)
    file(GLOB BENCH_SOURCES bench/*.cpp)
    file(GLOB BENCH_PROTOS bench/*.proto)
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
        rapidjson
        nlohmann_json
        tomlplusplus::tomlplusplus
        protobuf::libprotobuf-lite
        msgpack-cxx
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
        protobuf::libprotobuf-lite
        msgpack-cxx
    )

    if (PostgreSQL_FOUND)
        file(GLOB_RECURSE SRCS examples/6-postgres/*)
        add_executable(example_postgres ${SRCS})
        target_compile_features(example_postgres PRIVATE cxx_std_17)

        target_link_libraries(example_postgres PRIVATE
            cpx
            fmt
            PostgreSQL::PostgreSQL
        )
    endif()

    file(GLOB_RECURSE SRCS examples/7-openai/*)
    add_executable(example_openai ${SRCS})
    target_compile_features(example_openai PRIVATE cxx_std_17)

    target_link_libraries(example_openai PRIVATE
        cpx
        fmt
        yyjson
        httplib
        CLI11
    )

    file(GLOB_RECURSE SRCS examples/8-ollama/*)
    add_executable(example_ollama ${SRCS})
    target_compile_features(example_ollama PRIVATE cxx_std_17)

    target_link_libraries(example_ollama PRIVATE
        cpx
        fmt
        yyjson
        httplib
        CLI11
    )

    file(GLOB_RECURSE SRCS examples/9-anthropic/*)
    add_executable(example_anthropic ${SRCS})
    target_compile_features(example_anthropic PRIVATE cxx_std_17)

    target_link_libraries(example_anthropic PRIVATE
        cpx
        fmt
        yyjson
        httplib
        CLI11
    )
endif()
