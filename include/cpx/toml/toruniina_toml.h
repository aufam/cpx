#ifndef CPX_TOML_TORUNIINA_TOML_H
#define CPX_TOML_TORUNIINA_TOML_H

#include <cpx/toml/toml.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/reflect.h>
#include <cpx/extend.h>
#include <array>
#include <variant>
#include <tuple>

#ifndef TOML11_TOML_HPP
#    include <toml.hpp>
#endif

namespace __toml11 = ::toml;

namespace cpx::toml::toruniina_toml {
    using spec = __toml11::spec;

    template <typename From>
    using Serialize = cpx::serde::Serialize<__toml11::value, From>;

    template <typename To>
    using Deserialize = cpx::serde::Deserialize<__toml11::value, To>;

    template <typename From>
    using Parse = cpx::serde::Parse<__toml11::value, From>;

    template <typename To>
    using Dump = cpx::serde::Dump<__toml11::value, To>;

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
} // namespace cpx::toml::toruniina_toml

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

namespace cpx::serde {
    template <>
    struct Dump<__toml11::value, std::string> {
        ::cpx::toml::toruniina_toml::spec spec = ::cpx::toml::toruniina_toml::spec::default_version();

        template <typename T>
        std::string from(const T &v) const {
            __toml11::value val = Serialize<__toml11::value, T>{}.from(v);
            return __toml11::format(val, spec);
        }
    };

    template <>
    struct Parse<__toml11::value, std::istream> {
        std::istream                     &stream;
        ::cpx::toml::toruniina_toml::spec spec     = ::cpx::toml::toruniina_toml::spec::default_version();
        std::string                       filename = "";

        template <typename T>
        void into(T &val) const {
            __toml11::value tbl;

            try {
                try {
                    tbl = __toml11::parse(stream, filename, spec);
                } catch (std::exception &e) {
                    throw error(e.what());
                }
                Deserialize<__toml11::value, T>{tbl}.into(val);
            } catch (error &err) {
                if (!filename.empty())
                    err.path = filename;
                throw;
            }
        }
    };

    template <>
    struct Parse<__toml11::value, std::string> {
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
                Deserialize<__toml11::value, T>{tbl}.into(val);
            } catch (error &err) {
                if (src_is_path)
                    err.path = src;
                throw;
            }
        }
    };

    // bool
    template <>
    struct Serialize<__toml11::value, bool> {
        __toml11::value from(bool v) const {
            return {__toml11::value::boolean_type(v)};
        }
    };

    template <>
    struct Deserialize<__toml11::value, bool> {
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
    struct Serialize<__toml11::value, T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>> {
        __toml11::value from(T v) const {
            return __toml11::value(__toml11::value::integer_type(v));
        }
    };

    template <typename T>
    struct Deserialize<__toml11::value, T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>> {
        const __toml11::value &node;

        void into(T &v) const {
            if (node.is_integer())
                v = (T)node.as_integer();
            else
                throw type_mismatch_error("int", ::cpx::toml::toruniina_toml::detail::type(node));
        }
    };

    // float
    template <typename T>
    struct Serialize<__toml11::value, T, std::enable_if_t<std::is_floating_point_v<T>>> {
        __toml11::value from(T v) const {
            return __toml11::value(__toml11::value::floating_type(v));
        }
    };

    template <typename T>
    struct Deserialize<__toml11::value, T, std::enable_if_t<std::is_floating_point_v<T>>> {
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
    struct Serialize<__toml11::value, std::string> {
        __toml11::value from(const std::string &v) const {
            return {v};
        }

        __toml11::value from_raw(const std::string &v) const {
            std::istringstream iss(v);
            try {
                return __toml11::parse(iss);
            } catch (std::exception &e) {
                throw error(e.what());
            }
        }
    };

    template <>
    struct Serialize<__toml11::value, std::string_view> {
        __toml11::value from(std::string_view v) const {
            return {__toml11::value::string_type(v)};
        }

        __toml11::value from_raw(std::string_view v) const {
            return Serialize<__toml11::value, std::string>{}.from_raw(std::string(v));
        }
    };

    template <>
    struct Deserialize<__toml11::value, std::string> {
        const __toml11::value &node;

        void into(std::string &v) const {
            if (node.is_string())
                v = node.as_string();
            else
                throw type_mismatch_error("string", ::cpx::toml::toruniina_toml::detail::type(node));
        }

        void into_raw(std::string &v) const {
            if (node.is_table()) {
                std::ostringstream oss;
                oss << __toml11::format(node);
                v = oss.str();
            } else
                throw type_mismatch_error("table", ::cpx::toml::toruniina_toml::detail::type(node));
        }
    };

    // optional
    template <typename T>
    struct Serialize<
        __toml11::value,
        std::optional<T>,
        std::enable_if_t<std::is_default_constructible_v<T> && is_serializable_v<__toml11::value, T>>> {
        __toml11::value from(const std::optional<T> &v) const {
            Serialize<__toml11::value, T> ser = {};
            if (v.has_value())
                return ser.from(*v);
            else
                return ser.from(T{});
        }
    };

    template <typename T>
    struct Deserialize<
        __toml11::value,
        std::optional<T>,
        std::enable_if_t<std::is_default_constructible_v<T> && is_deserializable_v<__toml11::value, T>>> {
        const __toml11::value &node;

        void into(std::optional<T> &v) const {
            if (node.is_empty()) {
                v = std::nullopt;
                return;
            }
            v = T{};
            Deserialize<__toml11::value, T>{node}.into(*v);
        }
    };

    // array
    template <typename T, size_t N>
    struct Serialize<__toml11::value, std::array<T, N>, std::enable_if_t<is_serializable_v<__toml11::value, T>>> {
        __toml11::value from(const std::array<T, N> &v) const {
            __toml11::value arr = __toml11::array();
            arr.as_array(std::nothrow).reserve(N);
            for (auto &item : v)
                arr.push_back(Serialize<__toml11::value, T>{}.from(item));
            return arr;
        }
    };

    template <typename T, size_t N>
    struct Deserialize<__toml11::value, std::array<T, N>, std::enable_if_t<is_deserializable_v<__toml11::value, T>>> {
        const __toml11::value &node;

        void into(std::array<T, N> &v) const {
            if (!node.is_array())
                throw type_mismatch_error("array", ::cpx::toml::toruniina_toml::detail::type(node));

            const auto  &arr = node.as_array();
            const size_t n   = arr.size();
            if (n != N)
                throw size_mismatch_error(N, n);

            for (size_t i = 0; i < n; ++i)
                try {
                    Deserialize<__toml11::value, T>{arr[i]}.into(v[i]);
                } catch (error &e) {
                    e.add_context(i);
                    throw;
                }
        };
    };

    template <typename T, typename A>
    struct Serialize<__toml11::value, std::vector<T, A>, std::enable_if_t<is_serializable_v<__toml11::value, T>>> {
        __toml11::value from(const std::vector<T, A> &v) const {
            __toml11::value arr = __toml11::array();
            arr.as_array(std::nothrow).reserve(v.size());
            for (auto &item : v)
                arr.push_back(Serialize<__toml11::value, T>{}.from(item));
            return arr;
        }
    };

    template <typename T, typename A>
    struct Deserialize<
        __toml11::value,
        std::vector<T, A>,
        std::enable_if_t<std::is_default_constructible_v<T> && is_deserializable_v<__toml11::value, T>>> {
        const __toml11::value &node;

        void into(std::vector<T, A> &v) const {
            if (!node.is_array())
                throw type_mismatch_error("array", ::cpx::toml::toruniina_toml::detail::type(node));

            const auto  &arr = node.as_array();
            const size_t n   = arr.size();
            v.resize(n);
            for (size_t i = 0; i < n; ++i)
                try {
                    Deserialize<__toml11::value, T>{arr[i]}.into(v[i]);
                } catch (error &e) {
                    e.add_context(i);
                    throw;
                }
        }
    };

    template <typename... Ts>
    struct Serialize<__toml11::value, std::tuple<Ts...>> {
        __toml11::value from(const std::tuple<Ts...> &tpl) {
            auto flatten           = cpx::flatten(tpl);
            using Tpl              = decltype(flatten);
            constexpr bool  is_tbl = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;
            __toml11::value node   = is_tbl ? __toml11::value(__toml11::table()) : __toml11::value(__toml11::array());

            size_t idx = 0;
            tuple_for_each(flatten, [&](auto &item, const size_t) {
                const cpx::TagInfo &t       = cpx::toml::get_tag_info(item);
                auto               &v       = cpx::detail::get_underlying_value(item);
                using T                     = std::decay_t<decltype(v)>;
                constexpr bool serializable = cpx::serde::is_serializable_v<__toml11::value, T>;

                if (!serializable || (is_tbl && t.key == ""))
                    return;

                size_t i = idx++;
                if (t.omitempty && detail::is_empty_value(v) && is_tbl)
                    return;

                __toml11::value val;
                try {
                    if (t.noserde)
                        if constexpr (std::is_same_v<T, std::string>)
                            val = Serialize<__toml11::value, std::string>{}.from_raw(v);
                        else
                            throw error("field with tag `noserde` can only be serialized from std::string");
                    else {
                        if constexpr (serializable)
                            val = Serialize<__toml11::value, T>{}.from(v);
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
    struct Deserialize<__toml11::value, std::tuple<Ts...>> {
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

            size_t idx = 0;
            tuple_for_each(flattened, [&](auto &item, const size_t) {
                const cpx::TagInfo &t         = cpx::toml::get_tag_info(item);
                auto               &v         = detail::get_underlying_value(item);
                using T                       = std::decay_t<decltype(v)>;
                constexpr bool deserializable = cpx::serde::is_deserializable_v<__toml11::value, T>;

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
                if (val->is_empty() && t.skipmissing)
                    return;

                try {
                    if (t.noserde)
                        if constexpr (std::is_same_v<T, std::string>)
                            Deserialize<__toml11::value, std::string>{*val}.into_raw(v);
                        else
                            throw error("field with tag `noserde` can only be deserialized into std::string");
                    else {
                        if constexpr (is_deserializable_v<__toml11::value, T>)
                            Deserialize<__toml11::value, T>{*val}.into(v);
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
    struct Serialize<__toml11::value, std::variant<T...>> {
        __toml11::value from(const std::variant<T...> &v) const {
            return std::visit(
                [](const auto &var) { return Serialize<__toml11::value, std::decay_t<decltype(var)>>{}.from(var); }, v
            );
        }
    };

    template <typename... T>
    struct Deserialize<__toml11::value, std::variant<T...>, std::enable_if_t<(std::is_default_constructible_v<T> && ...)>> {
        const __toml11::value &node;

        void into(std::variant<T...> &v) const {
            bool        done = false;
            std::string type_names;
            (
                [&]() {
                    try {
                        if (!done) {
                            auto element = T{};
                            Deserialize<__toml11::value, T>{node}.into(element);
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
    struct Serialize<
        __toml11::value,
        std::unordered_map<std::string, T, H, P, A>,
        std::enable_if_t<is_serializable_v<__toml11::value, T>>> {
        __toml11::value from(const std::unordered_map<std::string, T, H, P, A> &v) const {
            __toml11::value node = __toml11::table();
            for (auto &[key, item] : v)
                node.as_table()[key] = Serialize<__toml11::value, T>{}.from(item);
            return node;
        }
    };

    template <typename T, typename H, typename P, typename A>
    struct Deserialize<
        __toml11::value,
        std::unordered_map<std::string, T, H, P, A>,
        std::enable_if_t<std::is_default_constructible_v<T> && is_deserializable_v<__toml11::value, T>>> {
        const __toml11::value &node;

        void into(std::unordered_map<std::string, T, H, P, A> &v) const {
            if (!node.is_table())
                throw type_mismatch_error("table", ::cpx::toml::toruniina_toml::detail::type(node));

            for (auto &[key, node] : node.as_table(std::nothrow)) {
                auto item = T{};
                try {
                    Deserialize<__toml11::value, T>{node}.into(item);
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
    struct Serialize<__toml11::value, std::tm> {
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
    struct Serialize<__toml11::value, std::timespec> {
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
                return Serialize<__toml11::value, std::tm>{}.from(tm, ts.tv_nsec);
            }

            return Serialize<__toml11::value, std::string>{}.from(cpx::ts_to_string(ts));
        }
    };

    template <>
    struct Deserialize<__toml11::value, std::tm> {
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
    struct Deserialize<__toml11::value, std::timespec> {
        const __toml11::value &node;

        void into(std::timespec &v) const {
            if (auto val = &node.as_string(std::nothrow); node.is_string()) {
                v = cpx::ts_from_string(*val);
            } else {
                std::tm tm = {};
                Deserialize<__toml11::value, std::tm>{node}.into(tm, &v.tv_nsec);
                v.tv_sec = timegm(&tm);
            }
        }
    };

    // generic reflection
    template <typename T>
    struct Serialize<__toml11::value, T, std::enable_if_t<cpx::toml::has_reflect_v<T>>> {
        __toml11::value from(const T &v) const {
            return Serialize<__toml11::value, cpx::toml::const_reflect_t<T>>{}.from(cpx::toml::reflect_of(v));
        }
    };

    template <typename T>
    struct Deserialize<__toml11::value, T, std::enable_if_t<cpx::toml::has_reflect_v<T>>> {
        const __toml11::value &node;

        void into(T &v) const {
            decltype(auto) r = cpx::toml::reflect_of(v);
            cpx::serde::Deserialize<__toml11::value, cpx::toml::reflect_t<T>>{node}.into(r);
        }
    };
} // namespace cpx::serde

namespace cpx::toml::toruniina_toml {
    template <typename T>
    void parse(const std::string &str, T &val, const spec &spec) {
        cpx::toml::toruniina_toml::Parse<std::string>{str, spec}.into(val, false);
    }

    template <typename T>
    void parse(std::istream &stream, T &val, const spec &spec, const std::string &filename) {
        cpx::toml::toruniina_toml::Parse<std::istream>{stream, spec, filename}.into(val, false);
    }

    template <typename T>
    void parse_from_file(const std::string &path, T &val, const spec &spec) {
        cpx::toml::toruniina_toml::Parse<std::string>{path, spec}.into(val, true);
    }

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(const std::string &str, const spec &spec) {
        T val = {};
        cpx::toml::toruniina_toml::Parse<std::string>{str, spec}.into(val, false);
        return val;
    }

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T>
    parse(std::istream &stream, const spec &spec, const std::string &filename) {
        T val = {};
        cpx::toml::toruniina_toml::Parse<std::istream>{stream, spec, filename}.into(val, false);
        return val;
    }

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse_from_file(const std::string &path, const spec &spec) {
        T val = {};
        cpx::toml::toruniina_toml::Parse<std::string>{path, spec}.into(val, true);
        return val;
    }

    template <typename T>
    [[nodiscard]]
    std::string dump(const T &val, const spec &spec) {
        return cpx::toml::toruniina_toml::Dump<std::string>{spec}.from(val);
    }
} // namespace cpx::toml::toruniina_toml
#endif
