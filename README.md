# cpx

**`cpx (cpp extra)`** is a lightweight, header-only C++17 utility library designed to make modern C++ development more ergonomic and composable.

---

## Features

- **Tag-based serialization/deserialization**  
  Unified interface for JSON, TOML, Protobuf, CLI, and SQL

- **Composable iterator utilities**  
  `iterate`, `enumerate`, `zip`, `map`, `drop`, `take`, `collect`

- **RAII utilities**  
  Scope-exit guard via `defer`

- **SQL query builder**

- **CLI argument parsing**

- **Concurrency primitives**  
  Channel, queue, and semaphore

- **Inference utilities**  
  Support for ONNX Runtime and OpenVINO

- **...and more**

---

## Philosophy

`cpx` is intentionally **non-opinionated** about third-party dependencies.

It does not bundle or enforce specific libraries, instead it provides adapters so you can integrate with tools you already use.

---

## Supported Integrations

- **Formatting**  
  [`fmt`](https://github.com/fmtlib/fmt)

- **CLI parsing**  
  [`CLI11`](https://github.com/CLIUtils/CLI11), [`cxxopts`](https://github.com/jarro2783/cxxopts)

- **JSON**  
  [`yyjson`](https://github.com/ibireme/yyjson), [`nlohmann::json`](https://github.com/nlohmann/json)

- **TOML**  
  [`toml11`](https://github.com/ToruNiina/toml11), [`toml++`](https://github.com/marzer/tomlplusplus)

- **Inference runtimes**  
  [ONNX Runtime](https://onnx.ai), [OpenVINO](https://openvino.ai)

- **Protobuf**  
  [Protocol Buffers](https://protobuf.dev)

Contributions are very much welcomed :)

---

## Examples

See [`examples/`](examples/) for runnable demos:

| Example                                      | Description                             |
| -------------------------------------------- | --------------------------------------- |
| [`0-basic`](examples/0-basic/main.cpp)       | Tag-based serialization/deserialization |
| [`1-struct`](examples/1-struct/main.cpp)     | Struct reflection (Boost.PFR)           |
| [`2-enum`](examples/2-enum/main.cpp)         | Enum reflection (magic_enum)            |
| [`3-cli`](examples/3-cli/main.cpp)           | CLI argument parsing                    |
| [`4-sql`](examples/4-sql/main.cpp)           | SQLite query builder                    |
| [`5-protobuf`](examples/5-protobuf/main.cpp) | Protobuf serialization                  |

For more detailed usage, check [`tests/`](tests/).
