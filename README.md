# cpx

A header-only C++17 utility library providing ergonomic wrappers and abstractions for common patterns.

## Features

- **`Result<T, E>`** — Rust-inspired result type with monadic operations (`and_then`, `transform`, `map_err`, `or_else`)
- **`optional` utilities** — Monadic helpers (`and_then`, `transform`, `or_else`) for `std::optional`
- **`Iter`** — Composable iterator adapter with `enumerate`, `zip`, `map`, `drop`, `take`, and `collect`
- **`defer`** — RAII scope-exit guard
- **`Tag` / serde** — Tag-based serialization/deserialization for JSON (nlohmann, yyjson), TOML (toml++, toml11), and formatting
- **`sql`** — SQLite3 query builder
- **`cli`** — CLI argument parsing (CLI11, cxxopts)
- **`multithreading`** — Channel, queue, and semaphore primitives
- **`proto`** — Protobuf helpers
- **`genai` / `inference`** — Generative AI / inference utilities

## Requirements

- C++17 compiler
- CMake ≥ 3.14

Optional (needed for tests/examples): SQLite3, Protobuf.

## Integration

`cpx` is a header-only interface library. Add it to your CMake project:

```cmake
# via CPM
CPMAddPackage("gh:aufam/cpx#main")
target_link_libraries(your_target PRIVATE cpx::cpx)
```

Or just add `include/` to your include path and `#include` the headers you need.

## Usage

See [`examples/`](examples/) for runnable demos:

| Example | Description |
|---------|-------------|
| [`0-basic`](examples/0-basic/) | Tag-based JSON/TOML parsing and formatting |
| [`1-struct`](examples/1-struct/) | Struct reflection with Boost.PFR |
| [`2-enum`](examples/2-enum/) | Enum reflection with magic_enum |
| [`3-cli`](examples/3-cli/) | CLI argument parsing |
| [`4-sql`](examples/4-sql/) | SQLite query builder |
| [`5-protobuf`](examples/5-protobuf/) | Protobuf integration |

See [`tests/`](tests/) for detailed usage of each component.

## Building Examples and Tests

```bash
cmake -B build -DCPX_BUILD_EXAMPLES=ON -DCPX_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```