#ifndef CPX_TOML_TORUNIINA_TOML_H
#define CPX_TOML_TORUNIINA_TOML_H

#include <cpx/toml/toml.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <array>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

#ifndef TOML11_TOML_HPP
#    include <toml.hpp>
#endif

namespace __toml11 = ::toml;

#define SERIALIZE(...)      cpx::serde::Serialize<__toml11::value, __VA_ARGS__>
#define DESERIALIZE(...)    cpx::serde::Deserialize<__toml11::value, __VA_ARGS__>
#define SERIALIZABLE(...)   cpx::serde::is_serializable_v<__toml11::value, __VA_ARGS__>
#define DESERIALIZABLE(...) cpx::serde::is_deserializable_v<__toml11::value, __VA_ARGS__>
#define DUMP(...)           cpx::serde::Dump<__toml11::value, __VA_ARGS__>
#define PARSE(...)          cpx::serde::Parse<__toml11::value, __VA_ARGS__>

namespace cpx::toml::toruniina_toml {
    using spec = __toml11::spec;

    template <typename From>
    using Serialize = SERIALIZE(From);

    template <typename To>
    using Deserialize = DESERIALIZE(To);

    template <typename From>
    constexpr bool is_serializable_v = SERIALIZABLE(From);

    template <typename To>
    constexpr bool is_deserializable_v = DESERIALIZABLE(To);

    template <typename From>
    using Parse = PARSE(From);

    template <typename To>
    using Dump = DUMP(To);

    template <typename T>
    void parse(const std::string &str, T &val, const spec &s = spec::default_version());

    template <typename T>
    void parse(std::istream &stream, T &val, const spec &s = spec::default_version(), const std::string &filename = "");

    template <typename T>
    void parse_from_file(const std::string &path, T &val, const spec &s = spec::default_version());

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T>
    parse(const std::string &str, const spec &s = spec::default_version());

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T>
    parse(std::istream &str, const spec &s = spec::default_version(), const std::string &filename = "");

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T>
    parse_from_file(const std::string &path, const spec &s = spec::default_version());

    template <typename T>
    [[nodiscard]]
    std::string dump(const T &val, const spec &s = spec::default_version());

    template <typename T>
    void dump(std::ostream &os, const T &val, const spec &s = spec::default_version());
} // namespace cpx::toml::toruniina_toml

namespace cpx {
    namespace toruniina_toml = cpx::toml::toruniina_toml;
}

namespace cpx::toml::toruniina_toml::detail {
    inline std::string type(const __toml11::value &val) {
        switch (val.type()) {
        case __toml11::value_t::empty:
            return "null";
        case __toml11::value_t::boolean:
            return "boolean";
        case __toml11::value_t::integer:
            return "integer";
        case __toml11::value_t::floating:
            return "floating";
        case __toml11::value_t::string:
            return "string";
        case __toml11::value_t::offset_datetime:
            return "offset_datetime";
        case __toml11::value_t::local_datetime:
            return "local_datetime";
        case __toml11::value_t::local_date:
            return "local_date";
        case __toml11::value_t::local_time:
            return "local_time";
        case __toml11::value_t::array:
            return "array";
        case __toml11::value_t::table:
            return "table";
        }
        return "unknown";
    }
} // namespace cpx::toml::toruniina_toml::detail

// bool
template <>
struct SERIALIZE(bool) {
    __toml11::value from(bool v) const {
        return {__toml11::value::boolean_type(v)};
    }
};

template <>
struct DESERIALIZE(bool) {
    const __toml11::value &node;

    void into(bool &v) const {
        if (node.is_boolean())
            v = node.as_boolean();
        else
            throw type_mismatch_error("bool", ::cpx::toml::toruniina_toml::detail::type(node));
    }
};

// int
template <typename T>
struct SERIALIZE(T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>) {
    __toml11::value from(T v) const {
        return __toml11::value(__toml11::value::integer_type(v));
    }
};

template <typename T>
struct SERIALIZE(T, std::enable_if_t<std::is_floating_point_v<T>>) {
    __toml11::value from(T v) const {
        return __toml11::value(__toml11::value::floating_type(v));
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<std::is_floating_point_v<T>>) {
    const __toml11::value &node;

    void into(T &v) const {
        if (node.is_floating())
            v = (T)node.as_floating();
        else
            throw type_mismatch_error("float", ::cpx::toml::toruniina_toml::detail::type(node));
    }
};

// string
template <>
struct SERIALIZE(std::string) {
    __toml11::value from(const std::string &v) const {
        return __toml11::value::string_view_type(v.data(), v.size());
    }

    __toml11::value from(std::string &&v) const {
        return __toml11::value::string_type(std::move(v));
    }

    __toml11::value from_raw(
        const std::string &v, const cpx::toml::toruniina_toml::spec &spec = cpx::toml::toruniina_toml::spec::default_version()
    ) const {
        std::istringstream iss(v);
        try {
            return __toml11::parse(iss, "unknown file", spec);
        } catch (std::exception &e) {
            throw error(e.what());
        }
    }
};

template <>
struct SERIALIZE(std::string_view) {
    __toml11::value from(std::string_view v) const {
        return {__toml11::value::string_view_type(v)};
    }

    __toml11::value from_raw(std::string_view v) const {
        return SERIALIZE(std::string){}.from_raw(std::string(v));
    }
};

template <>
struct DESERIALIZE(std::string) {
    const __toml11::value &node;

    void into(std::string &v) const {
        if (node.is_string())
            v = node.as_string();
        else
            throw type_mismatch_error("string", ::cpx::toml::toruniina_toml::detail::type(node));
    }

    void into_raw(
        std::string &v, const cpx::toml::toruniina_toml::spec &spec = cpx::toml::toruniina_toml::spec::default_version()
    ) const {
        if (node.is_table()) {
            std::ostringstream oss;
            oss << __toml11::format(node, spec);
            v = oss.str();
        } else
            throw type_mismatch_error("table", ::cpx::toml::toruniina_toml::detail::type(node));
    }
};

// optional
template <typename T>
struct SERIALIZE(std::optional<T>, std::enable_if_t<std::is_default_constructible_v<T> && SERIALIZABLE(T)>) {
    __toml11::value from(const std::optional<T> &v) const {
        SERIALIZE(T) ser = {};
        if (v.has_value())
            return ser.from(*v);
        else
            return ser.from(T{});
    }
};

template <typename T>
struct DESERIALIZE(std::optional<T>, std::enable_if_t<std::is_default_constructible_v<T> && DESERIALIZABLE(T)>) {
    const __toml11::value &node;

    void into(std::optional<T> &v) const {
        if (node.is_empty()) {
            v = std::nullopt;
            return;
        }
        v = T{};
        DESERIALIZE(T){node}.into(*v);
    }
};

// array
template <typename T, size_t N>
struct SERIALIZE(std::array<T, N>, std::enable_if_t<SERIALIZABLE(T)>) {
    template <typename Container>
    __toml11::value from_container(const Container &v) const {
        __toml11::value arr = __toml11::array();
        arr.as_array(std::nothrow).reserve(v.size());
        for (auto &item : v)
            arr.push_back(SERIALIZE(T){}.from(item));
        return arr;
    }

    __toml11::value from(const std::array<T, N> &v) const {
        return from_container(v);
    }
};

template <typename T, size_t N>
struct DESERIALIZE(std::array<T, N>, std::enable_if_t<DESERIALIZABLE(T)>) {
    const __toml11::value &node;

    template <typename Container, typename F>
    void into_container(Container &v, F &&on_size_mismatch) const {
        if (!node.is_array())
            throw type_mismatch_error("array", ::cpx::toml::toruniina_toml::detail::type(node));

        const auto  &arr = node.as_array();
        const size_t n   = arr.size();
        if (n != N)
            on_size_mismatch(n);

        for (size_t i = 0; i < n; ++i)
            try {
                DESERIALIZE(T){arr[i]}.into(v[i]);
            } catch (error &e) {
                e.add_context(i);
                throw;
            }
    }

    void into(std::array<T, N> &v) const {
        into_container(v, [](size_t got) { throw size_mismatch_error(N, got); });
    };
};

template <typename T, typename A>
struct SERIALIZE(std::vector<T, A>, std::enable_if_t<SERIALIZABLE(T)>) {
    __toml11::value from(const std::vector<T, A> &v) const {
        SERIALIZE(std::array<T, 1>){}.from_container(v);
    }
};

template <typename T, typename A>
struct DESERIALIZE(std::vector<T, A>, std::enable_if_t<std::is_default_constructible_v<T> && DESERIALIZABLE(T)>) {
    const __toml11::value &node;

    void into(std::vector<T, A> &v) const {
        DESERIALIZE(std::array<T, 1>){node}.into_container(v, [&v](size_t got) { v.resize(got); });
    }
};

template <typename... Ts>
struct SERIALIZE(std::tuple<Ts...>) {
    __toml11::value from(const std::tuple<Ts...> &tpl) {
        auto flatten           = cpx::flatten(tpl);
        using Tpl              = decltype(flatten);
        constexpr bool  is_tbl = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;
        __toml11::value node   = is_tbl ? __toml11::value(__toml11::table()) : __toml11::value(__toml11::array());

        auto   oneofs      = std::array<std::string_view, std::tuple_size_v<Tpl>>{};
        size_t oneof_count = 0;
        size_t idx         = 0;
        tuple_for_each(flatten, [&](auto &item, const size_t) {
            const cpx::TagInfo &t       = cpx::toml::get_tag_info(item);
            auto               &v       = cpx::detail::get_underlying_value(item);
            using T                     = std::decay_t<decltype(v)>;
            constexpr bool serializable = SERIALIZABLE(T);

            if (!serializable || (is_tbl && t.key == ""))
                return;

            size_t i = idx++;
            if ((t.omitempty || !t.oneof.empty()) && detail::is_empty_value(v) && is_tbl)
                return;

            if (!t.oneof.empty()) {
                for (size_t j = 0; j < oneof_count; ++j) {
                    if (oneofs[j] == t.oneof)
                        throw duplicate_oneof_error(t.oneof);
                }
                oneofs[oneof_count++] = t.oneof;
            }

            __toml11::value val;
            try {
                if (t.noserde)
                    if constexpr (std::is_same_v<T, std::string>)
                        val = SERIALIZE(std::string){}.from_raw(v);
                    else
                        throw error("field with tag `noserde` can only be serialized from std::string");
                else {
                    if constexpr (serializable)
                        val = SERIALIZE(T){}.from(v);
                }
            } catch (error &e) {
                if (is_tbl)
                    e.add_context(t.key);
                else
                    e.add_context(i);
                throw;
            }
            if (is_tbl)
                node.as_table(std::nothrow)[std::string(t.key)] = std::move(val);
            else
                node.as_array(std::nothrow).push_back(std::move(val));
        });

        return node;
    }
};

template <typename... Ts>
struct DESERIALIZE(std::tuple<Ts...>) {
    const __toml11::value &node;

    void into(std::tuple<Ts...> &tpl) const {
        auto flattened        = flatten(tpl);
        using Tpl             = decltype(flattened);
        constexpr bool is_tbl = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;

        if (!is_tbl && !node.is_array())
            throw type_mismatch_error("array", ::cpx::toml::toruniina_toml::detail::type(node));
        if (is_tbl && !node.is_table())
            throw type_mismatch_error("table", ::cpx::toml::toruniina_toml::detail::type(node));
        const __toml11::array *arr = &node.as_array(std::nothrow);
        const __toml11::table *tbl = &node.as_table(std::nothrow);

        auto   oneofs      = std::array<std::string_view, std::tuple_size_v<Tpl>>{};
        size_t oneof_count = 0;
        size_t idx         = 0;
        tuple_for_each(flattened, [&](auto &item, const size_t) {
            const cpx::TagInfo &t         = cpx::toml::get_tag_info(item);
            auto               &v         = detail::get_underlying_value(item);
            using T                       = std::decay_t<decltype(v)>;
            constexpr bool deserializable = DESERIALIZABLE(T);

            if (!deserializable || (is_tbl && t.key == ""))
                return;

            const size_t           i = idx++;
            const __toml11::value  empty;
            const __toml11::value *val = &empty;
            if (is_tbl) {
                if (auto it = tbl->find(std::string(t.key)); it != tbl->end())
                    val = &it->second;
            } else {
                if (i < arr->size())
                    val = &(*arr)[i];
            }
            if (val->is_empty() && (t.skipmissing || !t.oneof.empty()))
                return;

            if (!t.oneof.empty()) {
                for (size_t j = 0; j < oneof_count; ++j) {
                    if (oneofs[j] == t.oneof)
                        throw duplicate_oneof_error(t.oneof);
                }
                oneofs[oneof_count++] = t.oneof;
            }

            try {
                if (t.noserde)
                    if constexpr (std::is_same_v<T, std::string>)
                        DESERIALIZE(std::string){*val}.into_raw(v);
                    else
                        throw error("field with tag `noserde` can only be deserialized into std::string");
                else {
                    if constexpr (DESERIALIZABLE(T))
                        DESERIALIZE(T){*val}.into(v);
                }
            } catch (error &e) {
                if (is_tbl)
                    e.add_context(t.key);
                else
                    e.add_context(i);
                throw;
            }
        });
    }
};

template <typename... T>
struct SERIALIZE(std::variant<T...>, std::enable_if_t<(SERIALIZABLE(T) && ...)>) {
    __toml11::value from(const std::variant<T...> &v) const {
        return std::visit([](const auto &var) { return SERIALIZE(std::decay_t<decltype(var)>){}.from(var); }, v);
    }
};

template <typename... T>
struct DESERIALIZE(std::variant<T...>, std::enable_if_t<((std::is_default_constructible_v<T> && DESERIALIZABLE(T)) && ...)>) {
    const __toml11::value &node;

    void into(std::variant<T...> &v) const {
        bool        done = false;
        std::string type_names;
        (
            [&]() {
                try {
                    if (!done) {
                        auto element = T{};
                        DESERIALIZE(T){node}.into(element);
                        v    = std::move(element);
                        done = true;
                    }
                } catch (type_mismatch_error &e) {
                    type_names += e.expected_type + '|';
                }
            }(),
            ...
        );
        if (!done) {
            type_names.pop_back();
            throw type_mismatch_error(type_names, ::cpx::toml::toruniina_toml::detail::type(node));
        }
    }
};

// table
template <typename T, typename H, typename P, typename A>
struct SERIALIZE(std::unordered_map<std::string, T, H, P, A>, std::enable_if_t<SERIALIZABLE(T)>) {
    __toml11::value from(const std::unordered_map<std::string, T, H, P, A> &v) const {
        __toml11::value node = __toml11::table();
        for (auto &[key, item] : v)
            node.as_table()[key] = SERIALIZE(T){}.from(item);
        return node;
    }
};

template <typename T, typename H, typename P, typename A>
struct
    DESERIALIZE(std::unordered_map<std::string, T, H, P, A>, std::enable_if_t<std::is_default_constructible_v<T> && DESERIALIZABLE(T)>) {
    const __toml11::value &node;

    void into(std::unordered_map<std::string, T, H, P, A> &v) const {
        if (!node.is_table())
            throw type_mismatch_error("table", ::cpx::toml::toruniina_toml::detail::type(node));

        for (auto &[key, node] : node.as_table(std::nothrow)) {
            auto item = T{};
            try {
                DESERIALIZE(T){node}.into(item);
            } catch (error &e) {
                e.add_context(std::string_view(key));
                throw;
            }
            v.emplace(std::string(key), std::move(item));
        }
    }
};

// std::tm
template <>
struct SERIALIZE(std::tm) {
    __toml11::value from(const std::tm &tm, long long nanos = 0) const {
        __toml11::offset_datetime dt = {};
        if (tm.tm_mday > 0) {
            dt.date.year  = (int16_t)(tm.tm_year + 1900);
            dt.date.month = tm.tm_mon + 1;
            dt.date.day   = tm.tm_mday;
        }

        dt.time.hour       = tm.tm_hour;
        dt.time.minute     = tm.tm_min;
        dt.time.second     = tm.tm_sec;
        dt.time.nanosecond = nanos;

        if (tm.tm_mday <= 0) {
            return dt.time;
        }
        return dt;
    }
};

template <>
struct SERIALIZE(std::timespec) {
    __toml11::value from(const std::timespec &ts) const {
        constexpr auto ten_years = 24l * 3600 * 365;

        time_t    seconds     = ts.tv_sec;
        long long nanoseconds = ts.tv_nsec;
        seconds += nanoseconds / 1'000'000'000;
        nanoseconds %= 1'000'000'000;
        if (nanoseconds < 0) {
            nanoseconds += 1'000'000'000;
            --seconds;
        }

        if (seconds > ten_years || seconds < 0) {
            time_t  t  = ts.tv_sec;
            std::tm tm = *std::gmtime(&t);
            return SERIALIZE(std::tm){}.from(tm, ts.tv_nsec);
        }

        return SERIALIZE(std::string){}.from(cpx::ts_to_string(ts));
    }
};

template <>
struct DESERIALIZE(std::tm) {
    const __toml11::value &node;

    void into(std::tm &v, long *nanos = nullptr) const {
        if (auto val = &node.as_offset_datetime(std::nothrow); node.is_offset_datetime()) {
            to_tm(val->time, v, nanos);
            to_tm(val->date, v);
            std::time_t t = std::mktime(&v);
            t -= static_cast<std::time_t>(val->offset.hour * 60 + val->offset.minute - local_utc_offset_minutes(t)) * 60;
            v = *std::gmtime(&t);
        } else if (auto val = &node.as_local_datetime(std::nothrow); node.is_local_datetime()) {
            to_tm(val->time, v, nanos);
            to_tm(val->date, v);
            std::time_t t = std::mktime(&v);
            v             = *std::gmtime(&t);
        } else if (auto val = &node.as_local_time(std::nothrow); node.is_local_time())
            to_tm(*val, v, nanos);
        else if (auto val = &node.as_local_date(std::nothrow); node.is_local_date())
            to_tm(*val, v);
        else if (auto val = &node.as_string(std::nothrow); node.is_string())
            v = cpx::tm_from_string(*val, nullptr, nanos);
        else
            throw type_mismatch_error("time", ::cpx::toml::toruniina_toml::detail::type(node));
    }

    static int local_utc_offset_minutes(std::time_t t) {
        std::tm gmt   = *std::gmtime(&t);
        std::tm local = *std::localtime(&t);
        return static_cast<int>(std::difftime(std::mktime(&local), std::mktime(&gmt)) / 60);
    }

    static void to_tm(const __toml11::local_date &d, std::tm &tm) {
        tm.tm_year = d.year - 1900; // tm_year is years since 1900
        tm.tm_mon  = d.month - 1;   // tm_mon is 0–11
        tm.tm_mday = d.day;
    }

    static void to_tm(const __toml11::local_time &t, std::tm &tm, long *nanos) {
        tm.tm_hour = t.hour;
        tm.tm_min  = t.minute;
        tm.tm_sec  = t.second;
        if (nanos)
            *nanos = t.millisecond * 1000000 + t.microsecond * 1000 + t.nanosecond;
    }
};

template <>
struct DESERIALIZE(std::timespec) {
    const __toml11::value &node;

    void into(std::timespec &v) const {
        std::tm tm = {};
        DESERIALIZE(std::tm){node}.into(tm, &v.tv_nsec);
        v.tv_sec = timegm(&tm);
    }
};

// generic reflection
template <typename T>
struct SERIALIZE(T, std::enable_if_t<cpx::toml::has_reflect_v<T>>) {
    __toml11::value from(const T &v) const {
        return SERIALIZE(cpx::toml::const_reflect_t<T>){}.from(cpx::toml::reflect_of(v));
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<cpx::toml::has_reflect_v<T>>) {
    const __toml11::value &node;

    void into(T &v) const {
        decltype(auto) r = cpx::toml::reflect_of(v);
        DESERIALIZE(cpx::toml::reflect_t<T>){node}.into(r);
    }
};

// dump and parse
template <>
struct DUMP(std::ostream) {
    std::ostream                     &os;
    ::cpx::toml::toruniina_toml::spec spec = ::cpx::toml::toruniina_toml::spec::default_version();

    template <typename T>
    std::ostream &from(const T &v) const {
        __toml11::value val = SERIALIZE(T){}.from(v);
        return os << __toml11::format(val, spec);
    }

    template <typename T>
    std::ostream &operator<<(const T &v) const {
        return from(v);
    }
};

template <>
struct DUMP(std::string) {
    ::cpx::toml::toruniina_toml::spec spec = ::cpx::toml::toruniina_toml::spec::default_version();

    template <typename T>
    std::string from(const T &v) const {
        __toml11::value val = SERIALIZE(T){}.from(v);
        return __toml11::format(val, spec);
    }
};

template <>
struct PARSE(std::istream) {
    std::istream                     &stream;
    ::cpx::toml::toruniina_toml::spec spec     = ::cpx::toml::toruniina_toml::spec::default_version();
    std::string                       filename = "";

    template <typename T>
    std::istream &into(T &val) const {
        __toml11::value tbl;

        try {
            try {
                tbl = __toml11::parse(stream, filename, spec);
            } catch (std::exception &e) {
                throw error(e.what());
            }
            DESERIALIZE(T){tbl}.into(val);
        } catch (error &err) {
            if (!filename.empty())
                err.path = filename;
            throw;
        }
        return stream;
    }

    template <typename T>
    std::istream &operator>>(T &val) const {
        return into(val);
    }
};

template <>
struct PARSE(std::string) {
    const std::string                &src;
    ::cpx::toml::toruniina_toml::spec spec = ::cpx::toml::toruniina_toml::spec::default_version();

    template <typename T>
    void into(T &val, bool src_is_path = false) const {
        __toml11::value tbl;

        try {
            try {
                if (src_is_path) {
                    tbl = __toml11::parse(src, spec);
                } else {
                    std::istringstream iss(src);
                    tbl = __toml11::parse(iss, "<unknown>", spec);
                }
            } catch (std::exception &e) {
                throw error(e.what());
            }
            DESERIALIZE(T){tbl}.into(val);
        } catch (error &err) {
            if (src_is_path)
                err.path = src;
            throw;
        }
    }
};

template <typename T>
void cpx::toml::toruniina_toml::parse(const std::string &str, T &val, const spec &spec) {
    Parse<std::string>{str, spec}.into(val, false);
}

template <typename T>
void cpx::toml::toruniina_toml::parse(std::istream &stream, T &val, const spec &spec, const std::string &filename) {
    Parse<std::istream>{stream, spec, filename}.into(val, false);
}

template <typename T>
void cpx::toml::toruniina_toml::parse_from_file(const std::string &path, T &val, const spec &spec) {
    Parse<std::string>{path, spec}.into(val, true);
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T>
cpx::toml::toruniina_toml::parse(const std::string &str, const spec &spec) {
    T val = {};
    Parse<std::string>{str, spec}.into(val, false);
    return val;
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T>
cpx::toml::toruniina_toml::parse(std::istream &stream, const spec &spec, const std::string &filename) {
    T val = {};
    Parse<std::istream>{stream, spec, filename}.into(val, false);
    return val;
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T>
cpx::toml::toruniina_toml::parse_from_file(const std::string &path, const spec &spec) {
    T val = {};
    Parse<std::string>{path, spec}.into(val, true);
    return val;
}

template <typename T>
[[nodiscard]]
std::string cpx::toml::toruniina_toml::dump(const T &val, const spec &spec) {
    return Dump<std::string>{spec}.from(val);
}

template <typename T>
void cpx::toml::toruniina_toml::dump(std::ostream &os, const T &val, const spec &spec) {
    return Dump<std::ostream>{os, spec}.from(val);
}

namespace cpx::toml::toruniina_toml {
    constexpr struct IO {
        spec _spec = spec::default_version();

        constexpr IO spec(spec val) const {
            IO self    = *this;
            self._spec = val;
            return self;
        }

        friend Dump<std::ostream> operator<<(std::ostream &os, const IO &io) {
            return {os, io._spec};
        }

        friend Parse<std::istream> operator>>(std::istream &is, const IO &io) {
            return {is, io._spec};
        }
    } io{};
} // namespace cpx::toml::toruniina_toml

#undef SERIALIZE
#undef DESERIALIZE
#undef SERIALIZABLE
#undef DESERIALIZABLE
#undef DUMP
#undef PARSE
#endif
