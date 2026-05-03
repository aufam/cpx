module;

#include <cpx/sql/sqlite3.h>

export module cpx.sqlite;
import cpx;

export namespace cpx::sql::sqlite3 {
    using ::cpx::sql::sqlite3::Connection;
    using ::cpx::sql::sqlite3::Rows;

    using ::cpx::sql::sqlite3::Deserialize;
    using ::cpx::sql::sqlite3::Serialize;

    using ::cpx::sql::sqlite3::error;
} // namespace cpx::sql::sqlite3

export namespace cpx {
    namespace sqlite = ::cpx::sql::sqlite3;
}

#define SERIALIZER   ::cpx::sql::sqlite3::Serializer
#define DESERIALIZER ::cpx::sql::sqlite3::Deserializer

export {
    template <>
    struct cpx::serde::Serialize<SERIALIZER, int>;

    template <typename T>
    struct cpx::serde::Serialize<SERIALIZER, T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, int>>>;

    template <>
    struct cpx::serde::Serialize<SERIALIZER, std::tm>;

    template <>
    struct cpx::serde::Serialize<SERIALIZER, double>;

    template <>
    struct cpx::serde::Serialize<SERIALIZER, std::string>;

    template <>
    struct cpx::serde::Serialize<SERIALIZER, std::vector<uint8_t>>;

    template <typename T>
    struct cpx::serde::Serialize<SERIALIZER, std::optional<T>>;

    template <>
    struct cpx::serde::Deserialize<DESERIALIZER, int>;

    template <typename T>
    struct cpx::serde::Deserialize<DESERIALIZER, T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, int>>>;

    template <>
    struct cpx::serde::Deserialize<DESERIALIZER, std::tm>;

    template <>
    struct cpx::serde::Deserialize<DESERIALIZER, double>;

    template <>
    struct cpx::serde::Deserialize<DESERIALIZER, std::string>;

    template <>
    struct cpx::serde::Deserialize<DESERIALIZER, std::vector<uint8_t>>;

    template <typename T>
    struct cpx::serde::Deserialize<DESERIALIZER, std::optional<T>>;
}

#undef SERIALIZER
#undef DESERIALIZER
