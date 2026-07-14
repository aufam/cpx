#ifndef CPX_TOML_MARZER_TOML_H
#define CPX_TOML_MARZER_TOML_H

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

#ifndef TOMLPLUSPLUS_HPP
#    include <toml++/toml.h>
#endif

namespace __tomlpp = ::toml;

#define SERIALIZE(...)      cpx::serde::Serialize<__tomlpp::node, __VA_ARGS__>
#define DESERIALIZE(...)    cpx::serde::Deserialize<__tomlpp::node, __VA_ARGS__>
#define SERIALIZABLE(...)   cpx::serde::is_serializable_v<__tomlpp::node, __VA_ARGS__>
#define DESERIALIZABLE(...) cpx::serde::is_deserializable_v<__tomlpp::node, __VA_ARGS__>
#define DUMP(...)           cpx::serde::Dump<__tomlpp::table, __VA_ARGS__>
#define PARSE(...)          cpx::serde::Parse<__tomlpp::table, __VA_ARGS__>

namespace cpx::toml::marzer_toml {
    CPX_EXPORT template <typename From>
    using Serialize = SERIALIZE(From);

    CPX_EXPORT template <typename To>
    using Deserialize = DESERIALIZE(To);

    CPX_EXPORT template <typename From>
    constexpr bool is_serializable_v = SERIALIZABLE(From);

    CPX_EXPORT template <typename To>
    constexpr bool is_deserializable_v = DESERIALIZABLE(To);

    CPX_EXPORT template <typename From>
    using Parse = PARSE(From);

    CPX_EXPORT template <typename To>
    using Dump = DUMP(To);

    CPX_EXPORT template <typename T>
    void parse(const std::string &str, T &val);

    CPX_EXPORT template <typename T>
    void parse(std::istream &stream, T &val);

    CPX_EXPORT template <typename T>
    void parse_from_file(const std::string &path, T &val);

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(const std::string &str);

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(std::istream &stream);

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse_from_file(const std::string &path);

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::string dump(const T &val);

    CPX_EXPORT template <typename T>
    void dump(std::ostream &, const T &val);
} // namespace cpx::toml::marzer_toml

namespace cpx {
    CPX_EXPORT namespace marzer_toml = cpx::toml::marzer_toml;
}

// bool
template <>
struct SERIALIZE(bool) {
    std::unique_ptr<__tomlpp::node> from(bool v) const {
        return std::make_unique<__tomlpp::value<bool>>(v);
    }
};

template <>
struct DESERIALIZE(bool) {
    const __tomlpp::node *node;

    void into(bool &v) const {
        if (!node)
            throw type_mismatch_error("bool", "null");
        else if (auto val = node->as_boolean())
            v = val->get();
        else
            throw type_mismatch_error("bool", std::string(__tomlpp::impl::node_type_friendly_names[(int)node->type()]));
    }
};

// int
template <typename T>
struct SERIALIZE(T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>) {
    std::unique_ptr<__tomlpp::node> from(T v) const {
        return std::make_unique<__tomlpp::value<int64_t>>(v);
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>) {
    const __tomlpp::node *node;

    void into(T &v) const {
        if (!node)
            throw type_mismatch_error("int", "null");
        else if (auto val = node->as_integer())
            v = (T)val->get();
        else
            throw type_mismatch_error("int", std::string(__tomlpp::impl::node_type_friendly_names[(int)node->type()]));
    }
};

// float
template <typename T>
struct SERIALIZE(T, std::enable_if_t<std::is_floating_point_v<T>>) {
    std::unique_ptr<__tomlpp::node> from(T v) const {
        return std::make_unique<__tomlpp::value<double>>(v);
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<std::is_floating_point_v<T>>) {
    const __tomlpp::node *node;

    void into(T &v) const {
        if (!node)
            throw type_mismatch_error("float", "null");
        else if (auto val = node->as_floating_point())
            v = (T)val->get();
        else
            throw type_mismatch_error("float", std::string(__tomlpp::impl::node_type_friendly_names[(int)node->type()]));
    }
};

// string
template <>
struct SERIALIZE(std::string_view) {
    std::unique_ptr<__tomlpp::node> from(std::string_view v) const {
        return std::make_unique<__tomlpp::value<std::string>>(std::string(v));
    }

    std::unique_ptr<__tomlpp::node> from_raw(std::string_view v) const {
        __tomlpp::table tbl;
        try {
            tbl = __tomlpp::parse(v);
        } catch (std::exception &e) {
            throw error(e.what());
        }
        return std::make_unique<__tomlpp::table>(std::move(tbl));
    }
};

template <>
struct SERIALIZE(std::string) {
    std::unique_ptr<__tomlpp::node> from(const std::string &v) const {
        return std::make_unique<__tomlpp::value<std::string>>(v);
    }

    std::unique_ptr<__tomlpp::node> from_raw(const std::string &v) const {
        return SERIALIZE(std::string_view){}.from_raw(v);
    }
};

template <>
struct DESERIALIZE(std::string) {
    const __tomlpp::node *node;

    void into(std::string &v) const {
        if (!node)
            throw type_mismatch_error("string", "null");
        else if (auto val = node->as_string())
            v = val->get();
        else
            throw type_mismatch_error("string", std::string(__tomlpp::impl::node_type_friendly_names[(int)node->type()]));
    }

    void into_raw(std::string &v) const {
        if (!node)
            throw type_mismatch_error("table", "null");
        else if (auto tbl = node->as_table()) {
            std::ostringstream oss;
            oss << *tbl;
            v = oss.str();
        } else
            throw type_mismatch_error("table", std::string(__tomlpp::impl::node_type_friendly_names[(int)node->type()]));
    }
};

// optional
template <typename T>
struct SERIALIZE(std::optional<T>, std::enable_if_t<std::is_default_constructible_v<T> && SERIALIZABLE(T)>) {
    std::unique_ptr<__tomlpp::node> from(const std::optional<T> &v) const {
        if (v.has_value())
            return SERIALIZE(T){}.from(*v);
        else
            return SERIALIZE(T){}.from(T{});
    }
};

template <typename T>
struct DESERIALIZE(std::optional<T>, std::enable_if_t<std::is_default_constructible_v<T> && DESERIALIZABLE(T)>) {
    const __tomlpp::node *node;

    void into(std::optional<T> &v) const {
        if (!node) {
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
    std::unique_ptr<__tomlpp::node> from(const std::array<T, N> &v) const {
        auto arr = std::make_unique<__tomlpp::array>();
        arr->reserve(N);
        for (auto &item : v)
            arr->push_back(std::move(*SERIALIZE(T){}.from(item)));
        return arr;
    }
};

template <typename T, size_t N>
struct DESERIALIZE(std::array<T, N>, std::enable_if_t<DESERIALIZABLE(T)>) {
    const __tomlpp::node *node;

    void into(std::array<T, N> &v) const {
        if (!node)
            throw type_mismatch_error("array", "null");

        auto arr = node->as_array();
        if (!arr)
            throw type_mismatch_error("array", std::string(__tomlpp::impl::node_type_friendly_names[(int)node->type()]));

        const size_t n = arr->size();
        if (n != N)
            throw size_mismatch_error(N, n);

        for (size_t i = 0; i < n; ++i)
            try {
                DESERIALIZE(T){arr->get(i)}.into(v[i]);
            } catch (error &e) {
                e.add_context(i);
                throw;
            }
    };
};

template <typename T, typename A>
struct SERIALIZE(std::vector<T, A>, std::enable_if_t<SERIALIZABLE(T)>) {
    std::unique_ptr<__tomlpp::node> from(const std::vector<T, A> &v) const {
        auto arr = std::make_unique<__tomlpp::array>();
        arr->reserve(v.size());
        for (auto &item : v)
            arr->push_back(std::move(*SERIALIZE(T){}.from(item)));
        return arr;
    }
};

template <typename T, typename A>
struct DESERIALIZE(std::vector<T, A>, std::enable_if_t<std::is_default_constructible_v<T> && DESERIALIZABLE(T)>) {
    const __tomlpp::node *node;

    void into(std::vector<T, A> &v) const {
        if (!node)
            throw type_mismatch_error("array", "null");

        auto arr = node->as_array();
        if (!arr)
            throw type_mismatch_error("array", std::string(__tomlpp::impl::node_type_friendly_names[(int)node->type()]));

        const size_t n = arr->size();
        v.resize(n);
        for (size_t i = 0; i < n; ++i)
            try {
                DESERIALIZE(T){arr->get(i)}.into(v[i]);
            } catch (error &e) {
                e.add_context(i);
                throw;
            }
    }
};

// tuple
template <typename... Ts>
struct SERIALIZE(std::tuple<Ts...>) {
    std::unique_ptr<__tomlpp::node> from(const std::tuple<Ts...> &tpl) {
        auto flattened        = cpx::flatten(tpl);
        using Tpl             = decltype(flattened);
        constexpr bool is_tbl = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;

        std::unique_ptr<__tomlpp::node> node =
            is_tbl ? std::unique_ptr<__tomlpp::node>(new __tomlpp::table) : std::unique_ptr<__tomlpp::node>(new __tomlpp::array);

        auto   oneofs      = std::array<std::string_view, std::tuple_size_v<Tpl>>{};
        size_t oneof_count = 0;
        size_t idx         = 0;
        tuple_for_each(flattened, [&](auto &item, const size_t) {
            const cpx::TagInfo &t = cpx::toml::get_tag_info(item);
            auto               &v = cpx::detail::get_underlying_value(item);
            using T               = std::decay_t<decltype(v)>;

            if (!SERIALIZABLE(T) || (is_tbl && t.key == ""))
                return;

            size_t i = idx++;
            if ((t.omitempty || !t.oneof.empty()) && cpx::detail::is_empty_value(v) && is_tbl)
                return;

            if (!t.oneof.empty()) {
                for (size_t j = 0; j < oneof_count; ++j) {
                    if (oneofs[j] == t.oneof)
                        throw duplicate_oneof_error(t.oneof);
                }
                oneofs[oneof_count++] = t.oneof;
            }

            std::unique_ptr<__tomlpp::node> val;
            try {
                if (t.noserde)
                    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
                        val = SERIALIZE(std::string_view){}.from_raw(v);
                    else
                        throw error("field with tag `noserde` can only be serialized from std::string");
                else {
                    if constexpr (SERIALIZABLE(T))
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
                node->as_table()->insert_or_assign(t.key, std::move(*val));
            else
                node->as_array()->push_back(std::move(*val));
        });

        return node;
    }
};

template <typename... Ts>
struct DESERIALIZE(std::tuple<Ts...>) {
    const __tomlpp::node *node;

    void into(std::tuple<Ts...> &tpl) const {
        if (!node)
            throw type_mismatch_error("table|array", "null");

        auto flattened        = flatten(tpl);
        using Tpl             = decltype(flattened);
        constexpr bool is_tbl = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;

        auto arr = node->as_array();
        auto tbl = node->as_table();
        if (!is_tbl && !arr)
            throw type_mismatch_error("array", std::string(__tomlpp::impl::node_type_friendly_names[(int)node->type()]));
        if (is_tbl && !tbl)
            throw type_mismatch_error("table", std::string(__tomlpp::impl::node_type_friendly_names[(int)node->type()]));

        auto   oneofs      = std::array<std::string_view, std::tuple_size_v<Tpl>>{};
        size_t oneof_count = 0;
        size_t idx         = 0;
        tuple_for_each(flattened, [&](auto &item, const size_t) {
            const cpx::TagInfo &t = cpx::toml::get_tag_info(item);
            auto               &v = cpx::detail::get_underlying_value(item);
            using T               = std::decay_t<decltype(v)>;

            if (!DESERIALIZABLE(T) || (is_tbl && t.key == ""))
                return;

            const size_t          i   = idx++;
            const __tomlpp::node *val = is_tbl ? tbl->get(t.key) : arr->get(i);
            if (!val && (t.skipmissing || !t.oneof.empty()))
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
                        DESERIALIZE(std::string){val}.into_raw(v);
                    else
                        throw error("field with tag `noserde` can only be deserialized into std::string");
                else {
                    if constexpr (DESERIALIZABLE(T))
                        DESERIALIZE(T){val}.into(v);
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

// variant
template <typename... T>
struct SERIALIZE(std::variant<T...>, std::enable_if_t<(SERIALIZABLE(T) && ...)>) {
    std::unique_ptr<__tomlpp::node> from(const std::variant<T...> &v) const {
        return std::visit([](const auto &var) { return SERIALIZE(std::decay_t<decltype(var)>){}.from(var); }, v);
    }
};

template <typename... T>
struct DESERIALIZE(std::variant<T...>, std::enable_if_t<((std::is_default_constructible_v<T> && DESERIALIZABLE(T)) && ...)>) {
    const __tomlpp::node *node;

    void into(std::variant<T...> &v) const {
        try_for_each(v, std::index_sequence_for<T...>{});
    }

protected:
    template <size_t... I>
    void try_for_each(std::variant<T...> &v, std::index_sequence<I...>) const {
        bool        done = false;
        std::string type_names;
        (
            [&]() {
                using Elem = std::tuple_element_t<I, std::tuple<T...>>;
                try {
                    if (!done) {
                        auto element = Elem{};
                        DESERIALIZE(Elem){node}.into(element);
                        v    = std::move(element);
                        done = true;
                    }
                } catch (type_mismatch_error &e) {
                    type_names += e.expected_type + '|';
                }
            }(),
            ...);
        if (!done) {
            type_names.pop_back();
            throw type_mismatch_error(type_names, std::string(__tomlpp::impl::node_type_friendly_names[(int)node->type()]));
        }
    }
};

// table
template <typename T, typename H, typename P, typename A>
struct SERIALIZE(std::unordered_map<std::string, T, H, P, A>, std::enable_if_t<SERIALIZABLE(T)>) {
    std::unique_ptr<__tomlpp::node> from(const std::unordered_map<std::string, T, H, P, A> &v) const {
        std::unique_ptr<__tomlpp::node> node = std::make_unique<__tomlpp::table>();
        for (auto &[key, item] : v)
            node->as_table()->insert_or_assign(key, std::move(*SERIALIZE(T){}.from(item)));
        return node;
    }
};

template <typename T, typename H, typename P, typename A>
struct DESERIALIZE(
    std::unordered_map<std::string, T, H, P, A>, std::enable_if_t<std::is_default_constructible_v<T> && DESERIALIZABLE(T)>
) {
    const __tomlpp::node *node;

    void into(std::unordered_map<std::string, T, H, P, A> &v) const {
        if (!node)
            throw type_mismatch_error("table", "null");

        auto table = node->as_table();
        if (!table)
            throw type_mismatch_error("table", std::string(__tomlpp::impl::node_type_friendly_names[(int)node->type()]));

        for (auto [key, node] : *table) {
            auto item = T{};
            try {
                DESERIALIZE(T){&node}.into(item);
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
    std::unique_ptr<__tomlpp::node> from(const std::tm &tm, long nanos = 0) const {
        __tomlpp::date_time dt = {};

        if (tm.tm_mday > 0) {
            dt.date.year  = tm.tm_year + 1900;
            dt.date.month = tm.tm_mon + 1;
            dt.date.day   = tm.tm_mday;
        }

        dt.time.hour       = tm.tm_hour;
        dt.time.minute     = tm.tm_min;
        dt.time.second     = tm.tm_sec;
        dt.time.nanosecond = nanos;
        dt.offset          = __tomlpp::time_offset{};

        if (tm.tm_mday <= 0) {
            return std::make_unique<__tomlpp::value<__tomlpp::time>>(dt.time);
        }
        return std::make_unique<__tomlpp::value<__tomlpp::date_time>>(dt);
    }
};

template <>
struct DESERIALIZE(std::tm) {
    const __tomlpp::node *node;

    void into(std::tm &v, long *nanos = nullptr) const {
        if (!node) {
            throw type_mismatch_error("time", "null");
        } else if (auto val = node->as_date_time()) {
            to_tm(val->get().time, v, nanos);
            to_tm(val->get().date, v);
            std::time_t t = std::mktime(&v);
            if (auto &off = val->get().offset; off.has_value())
                t -= static_cast<std::time_t>(off->minutes - local_utc_offset_minutes(t)) * 60;
            v = *std::gmtime(&t);
        } else if (auto val = node->as_date()) {
            to_tm(val->get(), v);
        } else if (auto val = node->as_time()) {
            to_tm(val->get(), v, nanos);
        } else if (auto val = node->as_string()) {
            v = cpx::tm_from_string(val->get(), nullptr, nanos);
        } else
            throw type_mismatch_error("time", std::string(__tomlpp::impl::node_type_friendly_names[(int)node->type()]));
    }

    static int local_utc_offset_minutes(std::time_t t) {
        std::tm gmt   = *std::gmtime(&t);
        std::tm local = *std::localtime(&t);
        return static_cast<int>(std::difftime(std::mktime(&local), std::mktime(&gmt)) / 60);
    }

    static void to_tm(const __tomlpp::date &d, std::tm &tm) {
        tm.tm_year = d.year - 1900; // tm_year is years since 1900
        tm.tm_mon  = d.month - 1;   // tm_mon is 0–11
        tm.tm_mday = d.day;
    }

    static void to_tm(const __tomlpp::time &t, std::tm &tm, long *nanos) {
        tm.tm_hour = t.hour;
        tm.tm_min  = t.minute;
        tm.tm_sec  = t.second;
        if (nanos)
            *nanos = t.nanosecond;
    }
};

// timespce
template <>
struct SERIALIZE(std::timespec) {
    std::unique_ptr<__tomlpp::node> from(const std::timespec &ts) const {
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
struct DESERIALIZE(std::timespec) {
    const __tomlpp::node *node;

    void into(std::timespec &v) const {
        if (!node) {
            throw type_mismatch_error("time", "null");
        } else {
            std::tm tm = {};
            DESERIALIZE(std::tm){node}.into(tm, &v.tv_nsec);
            v.tv_sec = timegm(&tm);
        }
    }
};

// generic reflection
template <typename T>
struct SERIALIZE(T, std::enable_if_t<cpx::toml::has_reflect_v<T>>) {
    std::unique_ptr<__tomlpp::node> from(const T &v) const {
        using traits = cpx::toml::reflect_traits<T>;
        if constexpr (traits::has_to_str) {
            std::string str;
            traits::to_str(v, str);
            return SERIALIZE(std::string){}.from(str);
        } else {
            return SERIALIZE(typename traits::const_type){}.from(traits::of(v));
        }
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<cpx::toml::has_reflect_v<T>>) {
    const __tomlpp::node *node;

    void into(T &v) const {
        using traits = cpx::toml::reflect_traits<T>;
        if constexpr (traits::has_from_str) {
            std::string r;
            DESERIALIZE(std::string){node}.into(r);
            traits::from_str(v, r);
        } else {
            decltype(auto) r = traits::of(v);
            DESERIALIZE(typename traits::type){node}.into(r);
        }
    }
};

// dump and parse
template <>
struct DUMP(std::ostream) {
    std::ostream &os;

    template <typename T>
    std::ostream &from(const T &v) const {
        auto val = SERIALIZE(T){}.from(v);
        if (__tomlpp::table *tbl = val->as_table())
            return os << *tbl;
        else
            throw type_mismatch_error("table", std::string(__tomlpp::impl::node_type_friendly_names[(int)val->type()]));
    }

    template <typename T>
    std::ostream &operator<<(const T &v) const {
        return from(v);
    }
};

template <>
struct DUMP(std::string) {
    template <typename T>
    std::string from(const T &v) const {
        std::unique_ptr<__tomlpp::node> val = SERIALIZE(T){}.from(v);
        if (__tomlpp::table *tbl = val->as_table()) {
            std::ostringstream oss;
            oss << *tbl;
            return oss.str();
        } else
            throw type_mismatch_error("table", std::string(__tomlpp::impl::node_type_friendly_names[(int)val->type()]));
    }
};

template <>
struct PARSE(std::istream) {
    std::istream &stream;
    std::string   filename = "";

    template <typename T>
    std::istream &into(T &val) const {
        __tomlpp::table tbl;

        try {
            try {
                tbl = __tomlpp::parse(stream);
            } catch (std::exception &e) {
                throw error(e.what());
            }
            __tomlpp::node *node = &tbl;
            DESERIALIZE(T){node}.into(val);
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
    const std::string &src;

    template <typename T>
    void into(T &val, bool src_is_path = false) const {
        __tomlpp::table tbl;

        try {
            try {
                tbl = src_is_path ? __tomlpp::parse_file(src) : __tomlpp::parse(src);
            } catch (std::exception &e) {
                throw error(e.what());
            }
            __tomlpp::node *node = &tbl;
            DESERIALIZE(T){node}.into(val);
        } catch (error &err) {
            if (src_is_path)
                err.path = src;
            throw;
        }
    }
};

template <typename T>
void cpx::toml::marzer_toml::parse(const std::string &str, T &val) {
    Parse<std::string>{str}.into(val);
}

template <typename T>
void cpx::toml::marzer_toml::parse(std::istream &stream, T &val) {
    Parse<std::istream>{stream}.into(val);
}

template <typename T>
void cpx::toml::marzer_toml::parse_from_file(const std::string &path, T &val) {
    Parse<std::string>{path}.into(val, true);
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T> cpx::toml::marzer_toml::parse(const std::string &str) {
    T val = {};
    Parse<std::string>{str}.into(val);
    return val;
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T> cpx::toml::marzer_toml::parse(std::istream &stream) {
    T val = {};
    Parse<std::istream>{stream}.into(val);
    return val;
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T> cpx::toml::marzer_toml::parse_from_file(const std::string &path) {
    T val = {};
    Parse<std::string>{path}.into(val, true);
    return val;
}

template <typename T>
[[nodiscard]]
std::string cpx::toml::marzer_toml::dump(const T &val) {
    return Dump<std::string>{}.from(val);
}

template <typename T>
void cpx::toml::marzer_toml::dump(std::ostream &os, const T &val) {
    Dump<std::ostream>{os}.from(val);
}

namespace cpx::toml::marzer_toml {
    CPX_EXPORT constexpr struct IO {
        friend DUMP(std::ostream) operator<<(std::ostream &os, const IO &) {
            return {os};
        }

        friend PARSE(std::istream) operator>>(std::istream &is, const IO &) {
            return {is};
        }
    } io{};
} // namespace cpx::toml::marzer_toml

#undef SERIALIZE
#undef DESERIALIZE
#undef SERIALIZABLE
#undef DESERIALIZABLE
#undef DUMP
#undef PARSE
#endif
