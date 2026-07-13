# cpx

**`cpx (cpp extra)`** is a lightweight, header-only C++17 utility library designed to make modern C++ development more ergonomic and composable.

---

## Features

- **Tag-based reflection**
  Unified interface for JSON, TOML, YAML, Protobuf, MessagePack, CLI

- **SQL query builder**

- **Composable iterator utilities**
  `iterate`, `enumerate`, `zip`, `map`, `drop`, `take`, `collect`

- **RAII utilities**
  Scope-exit guard via `defer`

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
  [`CLI11`](https://github.com/CLIUtils/CLI11)

- **Struct reflection**
  [`Boost.PFR`](https://github.com/boostorg/pfr)

- **Enum reflection**
  [`magic_enum`](https://github.com/Neargye/magic_enum)

- **JSON**
  [`yyjson`](https://github.com/ibireme/yyjson), [`nlohmann::json`](https://github.com/nlohmann/json), [`rapidjson`](https://github.com/Tencent/rapidjson)

- **TOML**
  [`toml11`](https://github.com/ToruNiina/toml11), [`toml++`](https://github.com/marzer/tomlplusplus)

- **YAML**
  [`yaml-cpp`](https://github.com/jbeder/yaml-cpp)

- **MessagePack**
  [`msgpack`](https://github.com/msgpack/msgpack-c)

- **Protobuf**
  [Protocol Buffers](https://protobuf.dev)

- **Inference runtimes**
  [ONNX Runtime](https://onnx.ai), [OpenVINO](https://openvino.ai)

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

### Static Reflection

You can define `__field_tags__` for your custom types intrusively:
```cpp
#include <cpx/reflect.h>

struct User {
    std::string name;
    int         age;
    std::tm     created_at;

    static constexpr auto __field_tags__ = std::make_tuple(
        cpx::field<&User::name>       = "name",
        cpx::field<&User::age>        = "age",
        cpx::field<&User::created_at> = "created-at"
    );
};
```

Or, you can specialize `Reflect<T>` for your custom types without littering your public API:
```cpp
// public API
struct User {
    std::string name;
    int         age;
    std::tm     created_at;
};

// private API
#include <cpx/reflect.h>

template <>
struct cpx::Reflect<User> {
    static constexpr auto field_tags = std::make_tuple(
        cpx::field<&User::name>       = "name",
        cpx::field<&User::age>        = "age",
        cpx::field<&User::created_at> = "created-at"
    );
};

```

That’s it. No macros, no codegen, no black magic, no public API pollution — just pure C++17.

> **Tip:**
>
> Enable `AlignConsecutiveAssignments` and `AlignConsecutiveDeclarations` in your `.clang-format`
> configuration to automatically align the field definitions for improved readability.

You can then use your favorite serializer library:

```cpp
#include <cpx/yaml/jbeder_yaml.h>

User u = {"Sucipto", 24, now()};

// dump `u` into string
std::string yaml = cpx::jbeder_yaml::dump(u);

// parse from string into `u`
cpx::jbeder_yaml::parse(yaml, u);
```

You can specialize reflection for a specific format.

For example, JSON may prefer camelCase while others use kebab-case:

```cpp
#include <cpx/json/json.h>

template <>
struct cpx::json::Reflect<User> {
    static constexpr auto field_tags = std::make_tuple(
        cpx::field<&User::name>       = "name",
        cpx::field<&User::age>        = "age",
        cpx::field<&User::created_at> = "createdAt"
    );
};
```

Now:

```cpp
cpx::jbeder_yaml::dump(u);
// name: Sucipto
// age: 24
// created-at: ...

cpx::nlohmann_json::dump(u);
// {"name":"Sucipto","age":24,"createdAt":"..."}
```

In this context, **reflection** may not mean what you expect.
It is closer to a literal reflection (or mirroring) of custom types into already known representations
such as **numbers**, **strings**, **tuples**, and **tagged references**.

`Reflect<T> {...}` is simply a convenient way to reflect a struct
(or a class with public fields) into a tuple of tagged references.

You can construct one manually:

```cpp
#include <cpx/fmt.h>

std::string name = "Sucipto";
int         age  = 24;

TagInfo tag_name = "name";
TagInfo tag_age  = "age";

auto u = std::make_tuple(
    cpx::tag_tie(name, tag_name),
    cpx::tag_tie(age, tag_age)
); // std::tuple<cpx::TagInfoFor<std::string&, cpx::TagInfo&>, cpx::TagInfoFor<int&, cpx::TagInfo&>>

fmt::println("{}", u); // (name="Sucipto", age=24)
```

As the name suggests, `Reflect<T>` is not limited to reflecting fields. It can also act as a mirror for primitive types.

For example:

```cpp
template <>
struct cpx::Reflect<MyString> {
    using const_type = std::string_view; // for serialization
    using type = std::string; // for deserialization

    static const_type of(const MyString& str) {
        return str.view();
    }

    static decltype(auto) of(MyString& str) {
        // if MyString has mutable reference to std::string
        std::string& ref = str.get_mut_str();
        return ref;

        // otherwise, you may need some kind of proxy type that
        // converts string at destructor
        struct Proxy {
            MyString& str;
            std::string proxy;

            ~Proxy() noexcept(false) {
                str = MyString::from_std(proxy);
            }

            operator std::string&() {
                return proxy;
            }
        };

        return Proxy{str};
    }
};
```

### Third-party library integration

`cpx` does not care on how your third-party libraries are organized.

If the header guard matches, cpx will not try to include the default path.

For example:
```cpp
#include "you/might/locate/nlohmann_json/in/weird/path/json.h"
#include "as/well/as/toml++.h"
#include <cpx/json/nlohmann_json.h>
#include <cpx/toml/marzer_toml.h>
```

For libraries with native ADL serializers (such as `fmt` or `nlohmann::json`), `cpx` provides automatic integration:
```cpp
fmt::println("{}", u);
nlohmann::json j = u;
```

For libraries with SAX streaming support (avoid building an intermediate DOM), cpx provides a streming interface:
```cpp
#include <cpx/json/rapid_json.h>
#include <iostream>

User u = {"Sucipto", 24, now()};
std::cout << cpx::rapid_json::io << u << std::endl;
```


> **Note**
>
> `cpx` is only tested against the third-party versions listed in `CMakeLists.txt`.


### Tag properties

`TagInfo` stores metadata associated with a reflected field.

By default:

```cpp
struct TagInfo {
    std::string_view key   = "";
    std::string_view oneof = "";

    // CLI
    std::string_view short_ = "";
    std::string_view env    = "";
    std::string_view help   = "";

    // Protobuf / MessagePack
    int field_number = 0;

    // General serialization behavior
    bool omitempty   = false; // when serializing, omit zeros, nullopt, and .empty()
    bool skipmissing = false; // when deserializing, skip missing field instead of throw
    bool noserde     = false;
    bool positional  = false; // for CLI

    // Protobuf encoding
    bool fixed  = false;
    bool zigzag = false;
    bool packed = true;
};
```

#### String literal syntax

You can construct `TagInfo` directly from a string literal:

```cpp
TagInfo key = "key,omitempty,skipmissing";
```

This is equivalent to:

```cpp id="w5mec4"
TagInfo key;
key.key         = "key";
key.omitempty   = true;
key.skipmissing = true;
```

#### Protobuf example

```cpp
TagInfo user =
    "user,"
    "field_number=1,"
    "omitempty,"
    "skipmissing,"
    "env=USER,"
    "help=Define a username";

TagInfo age =
    "age,"
    "field_number=2,"
    "omitempty,"
    "skipmissing,"
    "fixed,"
    "help=Specify the user age";
```

#### Oneof example

Fields can belong to a `oneof` group.

The unique identifier must be shared among all members of the group.

If oneof is specified, it also implies `omitempty` and `skipmissing`

```cpp
TagInfo circle    = "circle,    oneof=circle|rectangle"; // space after comma is ignored
TagInfo rectangle = "rectangle, oneof=circle|rectangle";
```


#### TagInfoBuilder

String literals are concise but may be typo-prone.

You can use `TagInfoBuilder` instead.

Both constructor throgh string literals and builder are constexpr friendly tho.

```cpp
TagInfo user = TagInfoBuilder("user")
    .field_number(1)
    .omitempty()
    .skipmissing()
    .env("USER")
    .help("Define a username");
```

