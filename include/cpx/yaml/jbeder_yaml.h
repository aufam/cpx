#ifndef CPX_YAML_JBEDER_YAML_H
#define CPX_YAML_JBEDER_YAML_H

#include <cpx/yaml/yaml.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/serde/error.h>
#include <cpx/time.h>
#include <array>
#include <variant>
#include <tuple>
#include <unordered_map>
#include <ctime>


#ifndef YAML_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#    include <yaml-cpp/yaml.h>
#endif

#ifndef BOOST_PFR_HPP
#    if __has_include(<boost/pfr.hpp>)
#        include <boost/pfr.hpp>
#    endif
#endif

#ifndef NEARGYE_MAGIC_ENUM_HPP
#    if __has_include(<magic_enum/magic_enum.hpp>)
#        include <magic_enum/magic_enum.hpp>
#    endif
#endif

namespace __yaml_cpp = ::YAML;

namespace cpx::yaml::jbeder_yaml {
    template <typename From>
    using Serialize = cpx::serde::Serialize<__yaml_cpp::Node, From>;

    template <typename To>
    using Deserialize = cpx::serde::Deserialize<__yaml_cpp::Node, To>;

    template <typename From>
    using Parse = cpx::serde::Parse<__yaml_cpp::Node, From>;

    template <typename To>
    using Dump = cpx::serde::Dump<__yaml_cpp::Node, To>;

    template <typename T>
    void parse(const std::string &str, T &val);

    template <typename T>
    void parse(std::istream &stream, T &val, const std::string &filename = "");

    template <typename T>
    void parse_from_file(const std::string &path, T &val);

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(const std::string &str);

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(std::istream &stream, const std::string &filename = "");

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse_from_file(const std::string &path);

    template <typename T>
    [[nodiscard]]
    std::string dump(const T &val);

    template <typename T>
    [[nodiscard]]
    __yaml_cpp::Node dump_to_stream(const T &val);
} // namespace cpx::yaml::jbeder_yaml

namespace cpx::yaml::jbeder_yaml::detail {
    inline std::string type(const __yaml_cpp::Node &val) {
        switch (val.Type()) {
        case __yaml_cpp::NodeType::Null:
            return "null";
        case __yaml_cpp::NodeType::Scalar:
            return "scalar";
        case __yaml_cpp::NodeType::Sequence:
            return "sequence";
        case __yaml_cpp::NodeType::Map:
            return "map";
        case __yaml_cpp::NodeType::Undefined:
            return "undefined";
        }
        return "unknown";
    }

    template <typename T>
    struct is_primitive_type : std::bool_constant<std::is_integral_v<T> || std::is_floating_point_v<T>> {};

    template <typename T>
    constexpr const char *stringify_primitive_type() {
        if constexpr (std::is_same_v<bool, T>)
            return "bool";
        else if constexpr (std::is_integral_v<T>)
            return "int";
        else if constexpr (std::is_floating_point_v<T>)
            return "float";
        else
            return "<unknown>";
    }
} // namespace cpx::yaml::jbeder_yaml::detail

namespace cpx::serde {
    template <>
    struct Dump<__yaml_cpp::Node, std::ostream> {
        template <typename T>
        __yaml_cpp::Node from(const T &v) const {
            return Serialize<__yaml_cpp::Node, T>{}.from(v);
        }
    };

    template <>
    struct Dump<__yaml_cpp::Node, std::string> {
        template <typename T>
        std::string from(const T &v) const {
            __yaml_cpp::Node   val = Serialize<__yaml_cpp::Node, T>{}.from(v);
            std::ostringstream oss;
            oss << val;
            return oss.str();
        }
    };

    template <>
    struct Parse<__yaml_cpp::Node, std::string> {
        const std::string &src;

        template <typename T>
        void into(T &val, bool src_is_path = false) const {
            try {
                try {
                    __yaml_cpp::Node val = src_is_path ? __yaml_cpp::LoadFile(src) : __yaml_cpp::Load(src);
                } catch (std::exception &e) {
                    throw error(e.what());
                }
                Deserialize<__yaml_cpp::Node, T>{val}.into(val);
            } catch (error &err) {
                if (src_is_path)
                    err.path = src;
                throw;
            }
        }
    };

    template <>
    struct Parse<__yaml_cpp::Node, std::istream> {
        std::istream    &stream;
        std::string_view filename = "";

        template <typename T>
        void into(T &val) const {
            try {
                try {
                    __yaml_cpp::Node val = __yaml_cpp::Load(stream);
                } catch (std::exception &e) {
                    throw error(e.what());
                }
                Deserialize<__yaml_cpp::Node, T>{val}.into(val);
            } catch (error &err) {
                if (!filename.empty())
                    err.path = std::string(filename);
                throw;
            }
        }
    };

    // bool, integers, floats
    template <typename T>
    struct Serialize<__yaml_cpp::Node, T, std::enable_if_t<::cpx::yaml::jbeder_yaml::detail::is_primitive_type<T>::value>> {
        __yaml_cpp::Node from(T v) const {
            return __yaml_cpp::Node(v);
        }
    };

    template <typename T>
    struct Deserialize<__yaml_cpp::Node, T, std::enable_if_t<::cpx::yaml::jbeder_yaml::detail::is_primitive_type<T>::value>> {
        const __yaml_cpp::Node &node;

        void into(T &v) const {
            try {
                v = node.template as<T>();
            } catch (__yaml_cpp::BadConversion &e) {
                throw type_mismatch_error(
                    ::cpx::yaml::jbeder_yaml::detail::stringify_primitive_type<T>(),
                    ::cpx::yaml::jbeder_yaml::detail::type(node),
                    e.what()
                );
            }
        }
    };

    // string
    template <typename A>
    struct Serialize<__yaml_cpp::Node, std::basic_string<char, std::char_traits<char>, A>> {
        __yaml_cpp::Node from(const std::basic_string<char, std::char_traits<char>, A> &v) const {
            if constexpr (std::is_same_v<std::allocator<char>, A>)
                return __yaml_cpp::Node(v);
            else
                return __yaml_cpp::Node(std::string(v.c_str(), v.size()));
        }

        __yaml_cpp::Node from_raw(const std::basic_string<char, std::char_traits<char>, A> &v) const {
            try {
                if constexpr (std::is_same_v<std::allocator<char>, A>)
                    return __yaml_cpp::Load(v);
                else
                    return __yaml_cpp::Load(std::string(v.c_str(), v.size()));
            } catch (std::exception &e) {
                throw error(e.what());
            }
        }
    };

    template <>
    struct Serialize<__yaml_cpp::Node, std::string_view> {
        __yaml_cpp::Node from(std::string_view v) const {
            return __yaml_cpp::Node(v);
        }

        __yaml_cpp::Node from_raw(std::string_view v) const {
            return Serialize<__yaml_cpp::Node, std::string>{}.from_raw(std::string(v));
        }
    };

    template <typename A>
    struct Deserialize<__yaml_cpp::Node, std::basic_string<char, std::char_traits<char>, A>> {
        const __yaml_cpp::Node &node;

        void into(std::basic_string<char, std::char_traits<char>, A> &v) const {
            if (node.IsScalar())
                if constexpr (std::is_same_v<A, std::allocator<char>>)
                    v = node.template as<std::string>();
                else {
                    auto str = node.template as<std::string>();
                    v.assign(str.c_str(), str.size());
                }
            else
                throw type_mismatch_error("string", ::cpx::yaml::jbeder_yaml::detail::type(node));
        }

        void into_raw(std::basic_string<char, std::char_traits<char>, A> &v) const {
            std::ostringstream oss;
            oss << node;
            if constexpr (std::is_same_v<A, std::allocator<char>>)
                v = oss.str();
            else {
                std::string str = oss.str();
                v.assign(str.c_str(), str.size());
            }
        }
    };

    // optional
    template <typename T>
    struct Serialize<
        __yaml_cpp::Node,
        std::optional<T>,
        std::enable_if_t<std::is_default_constructible_v<T> && is_serializable_v<__yaml_cpp::Node, T>>> {
        __yaml_cpp::Node from(const std::optional<T> &v) const {
            Serialize<__yaml_cpp::Node, T> ser = {};
            if (v.has_value())
                return ser.from(*v);
            else
                return __yaml_cpp::Node(__yaml_cpp::NodeType::Null);
        }
    };

    template <typename T>
    struct Deserialize<
        __yaml_cpp::Node,
        std::optional<T>,
        std::enable_if_t<std::is_default_constructible_v<T> && is_deserializable_v<__yaml_cpp::Node, T>>> {
        const __yaml_cpp::Node &node;

        void into(std::optional<T> &v) const {
            if (!node.IsDefined() || node.IsNull()) {
                v = std::nullopt;
                return;
            }
            v = T{};
            Deserialize<__yaml_cpp::Node, T>{node}.into(*v);
        }
    };

    // array
    template <typename T, size_t N>
    struct Serialize<__yaml_cpp::Node, std::array<T, N>, std::enable_if_t<is_serializable_v<__yaml_cpp::Node, T>>> {
        __yaml_cpp::Node from(const std::array<T, N> &v) const {
            __yaml_cpp::Node arr(__yaml_cpp::NodeType::Sequence);
            for (auto &item : v)
                arr.push_back(Serialize<__yaml_cpp::Node, T>{}.from(item));
            return arr;
        }
    };

    template <typename T, size_t N>
    struct Deserialize<__yaml_cpp::Node, std::array<T, N>, std::enable_if_t<is_deserializable_v<__yaml_cpp::Node, T>>> {
        const __yaml_cpp::Node &node;

        void into(std::array<T, N> &v) const {
            if (!node.IsSequence())
                throw type_mismatch_error("array", ::cpx::yaml::jbeder_yaml::detail::type(node));

            const auto  &arr = node;
            const size_t n   = arr.size();
            if (n != N)
                throw size_mismatch_error(N, n);

            for (size_t i = 0; i < n; ++i)
                try {
                    Deserialize<__yaml_cpp::Node, T>{arr[i]}.into(v[i]);
                } catch (error &e) {
                    e.add_context(i);
                    throw;
                }
        };
    };

    template <typename T, typename A>
    struct Serialize<__yaml_cpp::Node, std::vector<T, A>, std::enable_if_t<is_serializable_v<__yaml_cpp::Node, T>>> {
        __yaml_cpp::Node from(const std::vector<T, A> &v) const {
            __yaml_cpp::Node arr(__yaml_cpp::NodeType::Sequence);
            for (auto &item : v)
                arr.push_back(Serialize<__yaml_cpp::Node, T>{}.from(item));
            return arr;
        }
    };

    template <typename T, typename A>
    struct Deserialize<
        __yaml_cpp::Node,
        std::vector<T, A>,
        std::enable_if_t<std::is_default_constructible_v<T> && is_deserializable_v<__yaml_cpp::Node, T>>> {
        const __yaml_cpp::Node &node;

        void into(std::vector<T, A> &v) const {
            if (!node.IsSequence())
                throw type_mismatch_error("array", ::cpx::yaml::jbeder_yaml::detail::type(node));

            const auto  &arr = node;
            const size_t n   = arr.size();
            v.resize(n);
            for (size_t i = 0; i < n; ++i)
                try {
                    Deserialize<__yaml_cpp::Node, T>{arr[i]}.into(v[i]);
                } catch (error &e) {
                    e.add_context(i);
                    throw;
                }
        }
    };

    template <typename... Ts>
    struct Serialize<__yaml_cpp::Node, std::tuple<Ts...>> {
        __yaml_cpp::Node from(const std::tuple<Ts...> &tpl) {
            const TagInfoTuple<sizeof...(Ts)>         ti     = cpx::yaml::get_tag_info_from_tuple(tpl);
            const std::array<TagInfo, sizeof...(Ts)> &ts     = ti.ts;
            const bool                                is_obj = ti.is_obj;

            __yaml_cpp::Node node(is_obj ? __yaml_cpp::NodeType::Map : __yaml_cpp::NodeType::Sequence);

            tuple_for_each(tpl, [&](const auto &item, const size_t i) {
                const TagInfo &t = ts[i];
                const auto    &v = detail::get_underlying_value(item);
                using T          = std::decay_t<decltype(v)>;

                if (!is_serializable_v<__yaml_cpp::Node, T> || (is_obj && t.key == "") ||
                    (t.omitempty && detail::is_empty_value(v)))
                    return;

                __yaml_cpp::Node val;
                try {
                    if (t.noserde)
                        if constexpr (std::is_same_v<T, std::string>)
                            val = Serialize<__yaml_cpp::Node, std::string>{}.from_raw(v);
                        else
                            throw error("field with tag `noserde` can only be serialized from std::string");
                    else {
                        if constexpr (is_serializable_v<__yaml_cpp::Node, T>)
                            val = Serialize<__yaml_cpp::Node, T>{}.from(v);
                    }
                } catch (error &e) {
                    if (is_obj)
                        e.add_context(t.key);
                    else
                        e.add_context(i);
                    throw;
                }
                if (is_obj)
                    node[t.key] = val;
                else
                    node.push_back(val);
            });

            return node;
        }
    };

    template <typename... Ts>
    struct Deserialize<__yaml_cpp::Node, std::tuple<Ts...>> {
        const __yaml_cpp::Node &node;

        void into(std::tuple<Ts...> &tpl) const {
            const TagInfoTuple<sizeof...(Ts)>         ti     = cpx::yaml::get_tag_info_from_tuple(tpl);
            const std::array<TagInfo, sizeof...(Ts)> &ts     = ti.ts;
            const bool                                is_obj = ti.is_obj;

            if (!is_obj && !node.IsSequence())
                throw type_mismatch_error("array", ::cpx::yaml::jbeder_yaml::detail::type(node));
            if (is_obj && !node.IsMap())
                throw type_mismatch_error("map", ::cpx::yaml::jbeder_yaml::detail::type(node));
            const auto &arr = node;
            const auto &tbl = node;

            tuple_for_each(tpl, [&](auto &item, const size_t i) {
                const TagInfo &t = ts[i];
                auto          &v = detail::get_underlying_value(item);
                using T          = std::decay_t<decltype(v)>;

                if (is_obj && t.key == "")
                    return;

                __yaml_cpp::Node val;
                if (is_obj) {
                    if (auto it = tbl[t.key]; it.IsDefined())
                        val = it;
                } else {
                    if (i < arr.size())
                        val = arr[i];
                }
                if (!val.IsDefined() && t.skipmissing)
                    return;

                try {
                    if (t.noserde)
                        if constexpr (std::is_same_v<T, std::string>)
                            Deserialize<__yaml_cpp::Node, std::string>{val}.into_raw(v);
                        else
                            throw error("field with tag `noserde` can only be deserialized into std::string");
                    else {
                        if constexpr (is_deserializable_v<__yaml_cpp::Node, T>)
                            Deserialize<__yaml_cpp::Node, T>{val}.into(v);
                    }
                } catch (error &e) {
                    if (is_obj)
                        e.add_context(t.key);
                    else
                        e.add_context(i);
                    throw;
                }
            });
        }
    };

    template <typename... T>
    struct Serialize<__yaml_cpp::Node, std::variant<T...>> {
        __yaml_cpp::Node from(const std::variant<T...> &v) const {
            return std::visit(
                [](const auto &var) { return Serialize<__yaml_cpp::Node, std::decay_t<decltype(var)>>{}.from(var); }, v
            );
        }
    };

    template <typename... T>
    struct Deserialize<__yaml_cpp::Node, std::variant<T...>, std::enable_if_t<(std::is_default_constructible_v<T> && ...)>> {
        const __yaml_cpp::Node &node;

        void into(std::variant<T...> &v) const {
            bool        done = false;
            std::string type_names;
            (
                [&]() {
                    try {
                        if (!done) {
                            auto element = T{};
                            Deserialize<__yaml_cpp::Node, T>{node}.into(element);
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
                throw type_mismatch_error(type_names, ::cpx::yaml::jbeder_yaml::detail::type(node));
            }
        }
    };

    // map
    template <typename T, typename H, typename P, typename A>
    struct Serialize<
        __yaml_cpp::Node,
        std::unordered_map<std::string, T, H, P, A>,
        std::enable_if_t<is_serializable_v<__yaml_cpp::Node, T>>> {
        __yaml_cpp::Node from(const std::unordered_map<std::string, T, H, P, A> &v) const {
            __yaml_cpp::Node node(__yaml_cpp::NodeType::Map);
            for (auto &[key, item] : v)
                node[key] = Serialize<__yaml_cpp::Node, T>{}.from(item);
            return node;
        }
    };

    template <typename T, typename H, typename P, typename A>
    struct Deserialize<
        __yaml_cpp::Node,
        std::unordered_map<std::string, T, H, P, A>,
        std::enable_if_t<std::is_default_constructible_v<T> && is_deserializable_v<__yaml_cpp::Node, T>>> {
        const __yaml_cpp::Node &node;

        void into(std::unordered_map<std::string, T, H, P, A> &v) const {
            if (!node.IsMap())
                throw type_mismatch_error("map", ::cpx::yaml::jbeder_yaml::detail::type(node));

            for (auto &kv : node) {
                const auto &key  = kv.first;
                const auto &node = kv.second;
                auto        item = T{};
                try {
                    Deserialize<__yaml_cpp::Node, T>{node}.into(item);
                } catch (error &e) {
                    e.add_context(key.Scalar());
                    throw;
                }
                v.emplace(key.Scalar(), std::move(item));
            }
        }
    };

    // std::tm
    template <>
    struct Serialize<__yaml_cpp::Node, std::tm> {
        __yaml_cpp::Node from(const std::tm &tm) const {
            return __yaml_cpp::Node(::cpx::tm_to_string(tm));
        }
    };

    template <>
    struct Deserialize<__yaml_cpp::Node, std::tm> {
        const __yaml_cpp::Node &node;

        void into(std::tm &v) const {
            std::string str;
            Deserialize<__yaml_cpp::Node, std::string>{node}.into(str);
            v = ::cpx::tm_from_string(str);
        }
    };

#ifdef BOOST_PFR_HPP
    template <typename S>
    struct Serialize<__yaml_cpp::Node, S, std::enable_if_t<std::is_aggregate_v<S> && !std::is_same_v<S, std::tm>>> {
        __yaml_cpp::Node from(const S &v) const {
            auto tpl = boost::pfr::structure_tie(v);
            return Serialize<__yaml_cpp::Node, decltype(tpl)>{}.from(tpl);
        }
    };

    template <typename S>
    struct Deserialize<__yaml_cpp::Node, S, std::enable_if_t<std::is_aggregate_v<S> && !std::is_same_v<S, std::tm>>> {
        const __yaml_cpp::Node &node;

        void into(S &v) const {
            auto tpl = boost::pfr::structure_tie(v);
            Deserialize<__yaml_cpp::Node, decltype(tpl)>{node}.into(tpl);
        }
    };
#endif

#ifdef NEARGYE_MAGIC_ENUM_HPP
    // enum
    template <typename S>
    struct Serialize<__yaml_cpp::Node, S, std::enable_if_t<std::is_enum_v<S>>> {
        __yaml_cpp::Node from(const S &v) const {
            return Serialize<__yaml_cpp::Node, std::string_view>{}.from(magic_enum::enum_name(v));
        }
    };

    template <typename S>
    struct Deserialize<__yaml_cpp::Node, S, std::enable_if_t<std::is_enum_v<S>>> {
        const __yaml_cpp::Node &node;

        void into(S &v) const {
            auto str = std::string();
            Deserialize<__yaml_cpp::Node, std::string>{node}.into(str);

            auto e = magic_enum::enum_cast<S>(str);
            if (!e.has_value()) {
                std::string what = "invalid value `" + str + "`, expected one of {";
                for (std::string_view name : magic_enum::enum_names<S>()) {
                    what += std::string(name) + ",";
                }
                what += "}";
                throw error(std::move(what));
            }
            v = *e;
        }
    };
#endif
} // namespace cpx::serde

namespace cpx::yaml::jbeder_yaml {
    template <typename T>
    void parse(const std::string &str, T &val) {
        cpx::yaml::jbeder_yaml::Parse<std::string>{str}.into(val, false);
    }

    template <typename T>
    void parse(std::istream &stream, T &val, const std::string &filename) {
        cpx::yaml::jbeder_yaml::Parse<std::istream>{stream}.into(val, filename);
    }

    template <typename T>
    void parse_from_file(const std::string &path, T &val) {
        cpx::yaml::jbeder_yaml::Parse<std::string>{path}.into(val, true);
    }

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(const std::string &str) {
        T val = {};
        cpx::yaml::jbeder_yaml::Parse<std::string>{str}.into(val, false);
        return val;
    }

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(std::istream &stream, const std::string &filename) {
        T val = {};
        cpx::yaml::jbeder_yaml::Parse<std::istream>{stream}.into(val, filename);
        return val;
    }

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse_from_file(const std::string &path) {
        T val = {};
        cpx::yaml::jbeder_yaml::Parse<std::string>{path}.into(val, true);
        return val;
    }

    template <typename T>
    [[nodiscard]]
    std::string dump(const T &val) {
        return cpx::yaml::jbeder_yaml::Dump<std::string>{}.from(val);
    }

    template <typename T>
    [[nodiscard]]
    __yaml_cpp::Node dump_to_stream(const T &val) {
        return cpx::yaml::jbeder_yaml::Dump<std::ostream>{}.from(val);
    }
} // namespace cpx::yaml::jbeder_yaml
#endif
