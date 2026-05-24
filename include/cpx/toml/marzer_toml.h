#ifndef CPX_TOML_MARZER_TOML_H
#define CPX_TOML_MARZER_TOML_H

#include <cpx/toml/toml.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/reflect.h>
#include <cpx/extend.h>
#include <array>
#include <variant>
#include <tuple>

#ifndef TOMLPLUSPLUS_HPP
#    include <toml++/toml.h>
#endif

namespace __tomlpp = ::toml;

namespace cpx::toml::marzer_toml {
    template <typename From>
    using Serialize = cpx::serde::Serialize<__tomlpp::node, From>;

    template <typename To>
    using Deserialize = cpx::serde::Deserialize<__tomlpp::node, To>;

    template <typename From>
    using Parse = cpx::serde::Parse<__tomlpp::table, From>;

    template <typename To>
    using Dump = cpx::serde::Dump<__tomlpp::table, To>;

    template <typename T>
    void parse(const std::string &str, T &val);

    template <typename T>
    void parse(std::istream &stream, T &val);

    template <typename T>
    void parse_from_file(const std::string &path, T &val);

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(const std::string &str);

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(std::istream &stream);

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse_from_file(const std::string &path);

    template <typename T>
    [[nodiscard]]
    std::string dump(const T &val);

    template <typename T>
    void dump(std::ostream &, const T &val);

} // namespace cpx::toml::marzer_toml


template <>
struct cpx::serde::Dump<__tomlpp::table, std::ostream> {
    std::ostream &os;

    template <typename T>
    std::ostream &from(const T &v) const {
        auto val = Serialize<__tomlpp::node, T>{}.from(v);
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
struct cpx::serde::Dump<__tomlpp::table, std::string> {
    template <typename T>
    std::string from(const T &v) const {
        auto val = Serialize<__tomlpp::node, T>{}.from(v);

        if (__tomlpp::table *tbl = val->as_table()) {
            std::ostringstream oss;
            oss << *tbl;
            return oss.str();
        } else
            throw type_mismatch_error("table", std::string(__tomlpp::impl::node_type_friendly_names[(int)val->type()]));
    }
};

template <>
struct cpx::serde::Parse<__tomlpp::table, std::istream> {
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
            Deserialize<__tomlpp::node, T>{node}.into(val);
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
struct cpx::serde::Parse<__tomlpp::table, std::string> {
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
            Deserialize<__tomlpp::node, T>{node}.into(val);
        } catch (error &err) {
            if (src_is_path)
                err.path = src;
            throw;
        }
    }
};

// bool
template <>
struct cpx::serde::Serialize<__tomlpp::node, bool> {
    std::unique_ptr<__tomlpp::node> from(bool v) const {
        return std::make_unique<__tomlpp::value<bool>>(v);
    }
};

template <>
struct cpx::serde::Deserialize<__tomlpp::node, bool> {
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
struct cpx::serde::Serialize<__tomlpp::node, T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>> {
    std::unique_ptr<__tomlpp::node> from(T v) const {
        return std::make_unique<__tomlpp::value<int64_t>>(v);
    }
};

template <typename T>
struct cpx::serde::Deserialize<__tomlpp::node, T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>> {
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
struct cpx::serde::Serialize<__tomlpp::node, T, std::enable_if_t<std::is_floating_point_v<T>>> {
    std::unique_ptr<__tomlpp::node> from(T v) const {
        return std::make_unique<__tomlpp::value<double>>(v);
    }
};

template <typename T>
struct cpx::serde::Deserialize<__tomlpp::node, T, std::enable_if_t<std::is_floating_point_v<T>>> {
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
struct cpx::serde::Serialize<__tomlpp::node, std::string_view> {
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
struct cpx::serde::Serialize<__tomlpp::node, std::string> {
    std::unique_ptr<__tomlpp::node> from(const std::string &v) const {
        return std::make_unique<__tomlpp::value<std::string>>(v);
    }

    std::unique_ptr<__tomlpp::node> from_raw(const std::string &v) const {
        return Serialize<__tomlpp::node, std::string_view>{}.from_raw(v);
    }
};

template <>
struct cpx::serde::Deserialize<__tomlpp::node, std::string> {
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
struct cpx::serde::Serialize<
    __tomlpp::node,
    std::optional<T>,
    std::enable_if_t<std::is_default_constructible_v<T> && cpx::serde::is_serializable_v<__tomlpp::node, T>>> {
    std::unique_ptr<__tomlpp::node> from(const std::optional<T> &v) const {
        Serialize<__tomlpp::node, T> ser = {};
        if (v.has_value())
            return ser.from(*v);
        else
            return ser.from(T{});
    }
};

template <typename T>
struct cpx::serde::Deserialize<
    __tomlpp::node,
    std::optional<T>,
    std::enable_if_t<std::is_default_constructible_v<T> && cpx::serde::is_deserializable_v<__tomlpp::node, T>>> {
    const __tomlpp::node *node;

    void into(std::optional<T> &v) const {
        if (!node) {
            v = std::nullopt;
            return;
        }
        v = T{};
        Deserialize<__tomlpp::node, T>{node}.into(*v);
    }
};

// array
template <typename T, size_t N>
struct cpx::serde::
    Serialize<__tomlpp::node, std::array<T, N>, std::enable_if_t<cpx::serde::is_serializable_v<__tomlpp::node, T>>> {
    std::unique_ptr<__tomlpp::node> from(const std::array<T, N> &v) const {
        auto arr = std::make_unique<__tomlpp::array>();
        arr->reserve(N);
        for (auto &item : v)
            arr->push_back(std::move(*Serialize<__tomlpp::node, T>{}.from(item)));
        return arr;
    }
};

template <typename T, size_t N>
struct cpx::serde::
    Deserialize<__tomlpp::node, std::array<T, N>, std::enable_if_t<cpx::serde::is_deserializable_v<__tomlpp::node, T>>> {
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
                Deserialize<__tomlpp::node, T>{arr->get(i)}.into(v[i]);
            } catch (error &e) {
                e.add_context(i);
                throw;
            }
    };
};

template <typename T, typename A>
struct cpx::serde::
    Serialize<__tomlpp::node, std::vector<T, A>, std::enable_if_t<cpx::serde::is_serializable_v<__tomlpp::node, T>>> {
    std::unique_ptr<__tomlpp::node> from(const std::vector<T, A> &v) const {
        auto arr = std::make_unique<__tomlpp::array>();
        arr->reserve(v.size());
        for (auto &item : v)
            arr->push_back(std::move(*Serialize<__tomlpp::node, T>{}.from(item)));
        return arr;
    }
};

template <typename T, typename A>
struct cpx::serde::Deserialize<
    __tomlpp::node,
    std::vector<T, A>,
    std::enable_if_t<std::is_default_constructible_v<T> && cpx::serde::is_deserializable_v<__tomlpp::node, T>>> {
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
                Deserialize<__tomlpp::node, T>{arr->get(i)}.into(v[i]);
            } catch (error &e) {
                e.add_context(i);
                throw;
            }
    }
};

template <typename... Ts>
struct cpx::serde::Serialize<__tomlpp::node, std::tuple<Ts...>> {
    std::unique_ptr<__tomlpp::node> from(const std::tuple<Ts...> &tpl) {
        auto flattened        = cpx::flatten(tpl);
        using Tpl             = decltype(flattened);
        constexpr bool is_tbl = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;

        std::unique_ptr<__tomlpp::node> node =
            is_tbl ? std::unique_ptr<__tomlpp::node>(new __tomlpp::table) : std::unique_ptr<__tomlpp::node>(new __tomlpp::array);

        size_t idx = 0;
        tuple_for_each(flattened, [&](auto &item, const size_t) {
            const cpx::TagInfo &t       = cpx::toml::get_tag_info(item);
            auto               &v       = cpx::detail::get_underlying_value(item);
            using T                     = std::decay_t<decltype(v)>;
            constexpr bool serializable = cpx::serde::is_serializable_v<__tomlpp::node, T>;

            if (!serializable || (is_tbl && t.key == ""))
                return;

            size_t i = idx++;
            if (t.omitempty && detail::is_empty_value(v) && is_tbl)
                return;

            std::unique_ptr<__tomlpp::node> val;
            try {
                if (t.noserde)
                    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
                        val = Serialize<__tomlpp::node, std::string_view>{}.from_raw(v);
                    else
                        throw error("field with tag `noserde` can only be serialized from std::string");
                else {
                    if constexpr (is_serializable_v<__tomlpp::node, T>)
                        val = Serialize<__tomlpp::node, T>{}.from(v);
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
struct cpx::serde::Deserialize<__tomlpp::node, std::tuple<Ts...>> {
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

        size_t idx = 0;
        tuple_for_each(flattened, [&](auto &item, const size_t) {
            const cpx::TagInfo &t         = cpx::toml::get_tag_info(item);
            auto               &v         = detail::get_underlying_value(item);
            using T                       = std::decay_t<decltype(v)>;
            constexpr bool deserializable = cpx::serde::is_deserializable_v<__tomlpp::node, T>;

            if (!deserializable || (is_tbl && t.key == ""))
                return;

            const size_t          i   = idx++;
            const __tomlpp::node *val = is_tbl ? tbl->get(t.key) : arr->get(i);
            if (!val && t.skipmissing)
                return;

            try {
                if (t.noserde)
                    if constexpr (std::is_same_v<T, std::string>)
                        Deserialize<__tomlpp::node, std::string>{val}.into_raw(v);
                    else
                        throw error("field with tag `noserde` can only be deserialized into std::string");
                else {
                    if constexpr (is_deserializable_v<__tomlpp::node, T>)
                        Deserialize<__tomlpp::node, T>{val}.into(v);
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
struct cpx::serde::Serialize<__tomlpp::node, std::variant<T...>> {
    std::unique_ptr<__tomlpp::node> from(const std::variant<T...> &v) const {
        return std::visit([](const auto &var) { return Serialize<__tomlpp::node, std::decay_t<decltype(var)>>{}.from(var); }, v);
    }
};

template <typename... T>
struct cpx::serde::
    Deserialize<__tomlpp::node, std::variant<T...>, std::enable_if_t<(std::is_default_constructible_v<T> && ...)>> {
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
                        Deserialize<__tomlpp::node, Elem>{node}.into(element);
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
struct cpx::serde::Serialize<
    __tomlpp::node,
    std::unordered_map<std::string, T, H, P, A>,
    std::enable_if_t<cpx::serde::is_serializable_v<__tomlpp::node, T>>> {
    std::unique_ptr<__tomlpp::node> from(const std::unordered_map<std::string, T, H, P, A> &v) const {
        std::unique_ptr<__tomlpp::node> node = std::make_unique<__tomlpp::table>();
        for (auto &[key, item] : v)
            node->as_table()->insert_or_assign(key, std::move(*Serialize<__tomlpp::node, T>{}.from(item)));
        return node;
    }
};

template <typename T, typename H, typename P, typename A>
struct cpx::serde::Deserialize<
    __tomlpp::node,
    std::unordered_map<std::string, T, H, P, A>,
    std::enable_if_t<std::is_default_constructible_v<T> && cpx::serde::is_deserializable_v<__tomlpp::node, T>>> {
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
                Deserialize<__tomlpp::node, T>{&node}.into(item);
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
struct cpx::serde::Serialize<__tomlpp::node, std::tm> {
    std::unique_ptr<__tomlpp::node> from(const std::tm &tm, long nanos) const {
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
struct cpx::serde::Deserialize<__tomlpp::node, std::tm> {
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

// generic reflection
template <typename T>
struct cpx::serde::Serialize<__tomlpp::node, T, std::enable_if_t<cpx::toml::has_reflect_v<T>>> {
    std::unique_ptr<__tomlpp::node> from(const T &v) const {
        return Serialize<__tomlpp::node, cpx::toml::const_reflect_t<T>>{}.from(cpx::toml::reflect_of(v));
    }
};

template <typename T>
struct cpx::serde::Deserialize<__tomlpp::node, T, std::enable_if_t<cpx::toml::has_reflect_v<T>>> {
    const __tomlpp::node *node;

    void into(T &v) const {
        decltype(auto) r = cpx::toml::reflect_of(v);
        cpx::serde::Deserialize<__tomlpp::node, cpx::toml::reflect_t<T>>{node}.into(r);
    }
};

template <typename T>
void cpx::toml::marzer_toml::parse(const std::string &str, T &val) {
    cpx::toml::marzer_toml::Parse<std::string>{str}.into(val);
}

template <typename T>
void cpx::toml::marzer_toml::parse(std::istream &stream, T &val) {
    cpx::toml::marzer_toml::Parse<std::istream>{stream}.into(val);
}

template <typename T>
void cpx::toml::marzer_toml::parse_from_file(const std::string &path, T &val) {
    cpx::toml::marzer_toml::Parse<std::string>{path}.into(val, true);
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T> cpx::toml::marzer_toml::parse(const std::string &str) {
    T val = {};
    cpx::toml::marzer_toml::Parse<std::string>{str}.into(val);
    return val;
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T> cpx::toml::marzer_toml::parse(std::istream &stream) {
    T val = {};
    cpx::toml::marzer_toml::Parse<std::istream>{stream}.into(val);
    return val;
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T> cpx::toml::marzer_toml::parse_from_file(const std::string &path) {
    T val = {};
    cpx::toml::marzer_toml::Parse<std::string>{path}.into(val, true);
    return val;
}

template <typename T>
[[nodiscard]]
std::string cpx::toml::marzer_toml::dump(const T &val) {
    return cpx::toml::marzer_toml::Dump<std::string>{}.from(val);
}

template <typename T>
void cpx::toml::marzer_toml::dump(std::ostream &os, const T &val) {
    cpx::toml::marzer_toml::Dump<std::ostream>{os}.from(val);
}

namespace cpx::toml::marzer_toml {
    constexpr struct IO {
        friend cpx::serde::Dump<__tomlpp::table, std::ostream> operator<<(std::ostream &os, const IO &) {
            return {os};
        }
        friend cpx::serde::Parse<__tomlpp::table, std::istream> operator>>(std::istream &is, const IO &) {
            return {is};
        }
    } io{};
} // namespace cpx::toml::marzer_toml
#endif
