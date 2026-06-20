#ifndef CPX_YAML_JBEDER_YAML_H
#define CPX_YAML_JBEDER_YAML_H

#include <cpx/yaml/yaml.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <array>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>


#ifndef YAML_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#    include <yaml-cpp/yaml.h>
#endif

namespace __yaml_cpp = ::YAML;

#define SERIALIZE(...)      cpx::serde::Serialize<__yaml_cpp::Node, __VA_ARGS__>
#define DESERIALIZE(...)    cpx::serde::Deserialize<__yaml_cpp::Node, __VA_ARGS__>
#define SERIALIZABLE(...)   cpx::serde::is_serializable_v<__yaml_cpp::Node, __VA_ARGS__>
#define DESERIALIZABLE(...) cpx::serde::is_deserializable_v<__yaml_cpp::Node, __VA_ARGS__>
#define DUMP(...)           cpx::serde::Dump<__yaml_cpp::Node, __VA_ARGS__>
#define PARSE(...)          cpx::serde::Parse<__yaml_cpp::Node, __VA_ARGS__>

#ifndef CPX_EXPORT
#    define CPX_EXPORT
#endif

namespace cpx::yaml::jbeder_yaml {
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
    void parse(std::istream &stream, T &val, const std::string &filename = "");

    CPX_EXPORT template <typename T>
    void parse_from_file(const std::string &path, T &val);

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(const std::string &str);

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(std::istream &stream, const std::string &filename = "");

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse_from_file(const std::string &path);

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::string dump(const T &val);

    CPX_EXPORT template <typename T>
    void dump(std::ostream &os, const T &val);
} // namespace cpx::yaml::jbeder_yaml

namespace cpx {
    CPX_EXPORT namespace jbeder_yaml = cpx::yaml::jbeder_yaml;
}

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

// bool, integers, floats
template <typename T>
struct SERIALIZE(T, std::enable_if_t<::cpx::yaml::jbeder_yaml::detail::is_primitive_type<T>::value>) {
    __yaml_cpp::Node from(T v) const {
        return __yaml_cpp::Node(v);
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<::cpx::yaml::jbeder_yaml::detail::is_primitive_type<T>::value>) {
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
struct SERIALIZE(std::basic_string<char, std::char_traits<char>, A>) {
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
struct SERIALIZE(std::string_view) {
    __yaml_cpp::Node from(std::string_view v) const {
        return __yaml_cpp::Node(v);
    }

    __yaml_cpp::Node from_raw(std::string_view v) const {
        return SERIALIZE(std::string){}.from_raw(std::string(v));
    }
};

template <typename A>
struct DESERIALIZE(std::basic_string<char, std::char_traits<char>, A>) {
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
struct SERIALIZE(std::optional<T>, std::enable_if_t<SERIALIZABLE(T)>) {
    __yaml_cpp::Node from(const std::optional<T> &v) const {
        Serialize<__yaml_cpp::Node, T> ser = {};
        if (v.has_value())
            return ser.from(*v);
        else
            return __yaml_cpp::Node(__yaml_cpp::NodeType::Null);
    }
};

template <typename T>
struct DESERIALIZE(std::optional<T>, std::enable_if_t<std::is_default_constructible_v<T> && DESERIALIZABLE(T)>) {
    const __yaml_cpp::Node &node;

    void into(std::optional<T> &v) const {
        if (!node.IsDefined() || node.IsNull()) {
            v = std::nullopt;
            return;
        }
        v.emplace();
        DESERIALIZE(T){node}.into(*v);
    }
};

// array
template <typename T, size_t N>
struct SERIALIZE(std::array<T, N>, std::enable_if_t<SERIALIZABLE(T)>) {
    template <typename Container>
    __yaml_cpp::Node from_container(const Container &v) const {
        __yaml_cpp::Node arr(__yaml_cpp::NodeType::Sequence);
        for (auto &item : v)
            arr.push_back(SERIALIZE(T){}.from(item));
        return arr;
    }

    __yaml_cpp::Node from(const std::array<T, N> &v) const {
        return from_container(v);
    }
};

template <typename T, size_t N>
struct DESERIALIZE(std::array<T, N>, std::enable_if_t<DESERIALIZABLE(T)>) {
    const __yaml_cpp::Node &node;

    template <typename Container, typename F>
    void into_container(Container &v, F &&on_size_mismatch) const {
        if (!node.IsSequence())
            throw type_mismatch_error("array", ::cpx::yaml::jbeder_yaml::detail::type(node));

        const auto  &arr = node;
        const size_t n   = arr.size();
        if (n != v.size())
            on_size_mismatch(n);

        for (size_t i = 0; i < n; ++i)
            try {
                Deserialize<__yaml_cpp::Node, T>{arr[i]}.into(v[i]);
            } catch (error &e) {
                e.add_context(i);
                throw;
            }
    };

    void into(std::array<T, N> &v) const {
        into_container(v, [](size_t got) { throw size_mismatch_error(N, got); });
    };
};

template <typename T, typename A>
struct SERIALIZE(std::vector<T, A>, std::enable_if_t<SERIALIZABLE(T)>) {
    __yaml_cpp::Node from(const std::vector<T, A> &v) const {
        return SERIALIZE(std::array<T, 1>){}.from_container(v);
    }
};

template <typename T, typename A>
struct DESERIALIZE(std::vector<T, A>, std::enable_if_t<std::is_default_constructible_v<T> && DESERIALIZABLE(T)>) {
    const __yaml_cpp::Node &node;

    void into(std::vector<T, A> &v) const {
        DESERIALIZE(std::array<T, 1>){node}.into_container(v, [&v](size_t got) { v.resize(got); });
    }
};

template <typename... Ts>
struct SERIALIZE(std::tuple<Ts...>) {
    __yaml_cpp::Node from(const std::tuple<Ts...> &tpl) {
        auto flattened        = cpx::flatten(tpl);
        using Tpl             = decltype(flattened);
        constexpr bool is_map = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;

        __yaml_cpp::Node node(is_map ? __yaml_cpp::NodeType::Map : __yaml_cpp::NodeType::Sequence);

        auto   oneofs      = std::array<std::string_view, std::tuple_size_v<Tpl>>{};
        size_t oneof_count = 0;
        size_t idx         = 0;
        tuple_for_each(flattened, [&](auto &item, const size_t) {
            const cpx::TagInfo &t       = cpx::yaml::get_tag_info(item);
            auto               &v       = cpx::detail::get_underlying_value(item);
            using T                     = std::decay_t<decltype(v)>;
            constexpr bool serializable = SERIALIZABLE(T);

            if (!serializable || (is_map && t.key == ""))
                return;

            size_t i = idx++;
            if ((t.omitempty || !t.oneof.empty()) && cpx::detail::is_empty_value(v)) {
                if constexpr (!is_map)
                    node.push_back(__yaml_cpp::Node(__yaml_cpp::NodeType::Null));
                return;
            }

            if (!t.oneof.empty()) {
                for (size_t j = 0; j < oneof_count; ++j) {
                    if (oneofs[j] == t.oneof)
                        throw duplicate_oneof_error(t.oneof);
                }
                oneofs[oneof_count++] = t.oneof;
            }

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
                if (is_map)
                    e.add_context(t.key);
                else
                    e.add_context(i);
                throw;
            }
            if (is_map)
                node[t.key] = val;
            else
                node.push_back(val);
        });

        return node;
    }
};

template <typename... Ts>
struct DESERIALIZE(std::tuple<Ts...>) {
    const __yaml_cpp::Node &node;

    void into(std::tuple<Ts...> &tpl) const {
        auto flattened        = flatten(tpl);
        using Tpl             = decltype(flattened);
        constexpr bool is_map = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;

        if (!is_map && !node.IsSequence())
            throw type_mismatch_error("array", ::cpx::yaml::jbeder_yaml::detail::type(node));
        if (is_map && !node.IsMap())
            throw type_mismatch_error("map", ::cpx::yaml::jbeder_yaml::detail::type(node));
        const auto &arr = node;
        const auto &map = node;

        auto   oneofs      = std::array<std::string_view, std::tuple_size_v<Tpl>>{};
        size_t oneof_count = 0;
        size_t idx         = 0;
        tuple_for_each(flattened, [&](auto &item, const size_t) {
            const cpx::TagInfo &t         = cpx::yaml::get_tag_info(item);
            auto               &v         = detail::get_underlying_value(item);
            using T                       = std::decay_t<decltype(v)>;
            constexpr bool deserializable = cpx::serde::is_deserializable_v<__yaml_cpp::Node, T>;

            if (!deserializable || (is_map && t.key == ""))
                return;

            const size_t     i = idx++;
            __yaml_cpp::Node val;
            if (is_map) {
                if (auto it = map[t.key]; it.IsDefined())
                    val = it;
            } else {
                if (i < arr.size())
                    val = arr[i];
            }
            if ((!val.IsDefined() || val.IsNull()) && (t.skipmissing || !t.oneof.empty()))
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
                        Deserialize<__yaml_cpp::Node, std::string>{val}.into_raw(v);
                    else
                        throw error("field with tag `noserde` can only be deserialized into std::string");
                else {
                    if constexpr (deserializable)
                        Deserialize<__yaml_cpp::Node, T>{val}.into(v);
                }
            } catch (error &e) {
                if (is_map)
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
    __yaml_cpp::Node from(const std::variant<T...> &v) const {
        return std::visit([](const auto &var) { return SERIALIZE(std::decay_t<decltype(var)>){}.from(var); }, v);
    }
};

template <typename... T>
struct DESERIALIZE(std::variant<T...>, std::enable_if_t<((std::is_default_constructible_v<T> && DESERIALIZABLE(T)) && ...)>) {
    const __yaml_cpp::Node &node;

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
            ...);
        if (!done) {
            type_names.pop_back();
            throw type_mismatch_error(type_names, ::cpx::yaml::jbeder_yaml::detail::type(node));
        }
    }
};

// map
template <typename T, typename H, typename P, typename A>
struct SERIALIZE(std::unordered_map<std::string, T, H, P, A>, std::enable_if_t<SERIALIZABLE(T)>) {
    __yaml_cpp::Node from(const std::unordered_map<std::string, T, H, P, A> &v) const {
        __yaml_cpp::Node node(__yaml_cpp::NodeType::Map);
        for (auto &[key, item] : v)
            node[key] = SERIALIZE(T){}.from(item);
        return node;
    }
};

template <typename T, typename H, typename P, typename A>
struct DESERIALIZE(
    std::unordered_map<std::string, T, H, P, A>, std::enable_if_t<std::is_default_constructible_v<T> && DESERIALIZABLE(T)>
) {
    const __yaml_cpp::Node &node;

    void into(std::unordered_map<std::string, T, H, P, A> &v) const {
        if (!node.IsMap())
            throw type_mismatch_error("map", ::cpx::yaml::jbeder_yaml::detail::type(node));

        for (auto &kv : node) {
            const auto &key  = kv.first;
            const auto &node = kv.second;
            auto        item = T{};
            try {
                DESERIALIZE(T){node}.into(item);
            } catch (error &e) {
                e.add_context(key.Scalar());
                throw;
            }
            v.emplace(key.Scalar(), std::move(item));
        }
    }
};

// generic reflection
template <typename T>
struct SERIALIZE(T, std::enable_if_t<cpx::yaml::has_reflect_v<T>>) {
    __yaml_cpp::Node from(const T &v) const {
        return SERIALIZE(cpx::yaml::const_reflect_t<T>){}.from(cpx::yaml::reflect_of(v));
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<cpx::yaml::has_reflect_v<T>>) {
    const __yaml_cpp::Node &node;

    void into(T &v) const {
        decltype(auto) r = cpx::yaml::reflect_of(v);
        DESERIALIZE(cpx::yaml::reflect_t<T>){node}.into(r);
    }
};

// dump and parse
template <>
struct DUMP(std::ostream) {
    std::ostream &os;

    template <typename T>
    std::ostream &from(const T &v) const {
        return os << SERIALIZE(T){}.from(v);
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
        __yaml_cpp::Node   val = SERIALIZE(T){}.from(v);
        std::ostringstream oss;
        oss << val;
        return oss.str();
    }
};

template <>
struct PARSE(std::string) {
    const std::string &src;

    template <typename T>
    void into(T &v, bool src_is_path = false) const {
        try {
            __yaml_cpp::Node val;
            try {
                val = src_is_path ? __yaml_cpp::LoadFile(src) : __yaml_cpp::Load(src);
            } catch (std::exception &e) {
                throw error(e.what());
            }
            DESERIALIZE(T){val}.into(v);
        } catch (error &err) {
            if (src_is_path)
                err.path = src;
            throw;
        }
    }
};

template <>
struct PARSE(std::istream) {
    std::istream    &stream;
    std::string_view filename = "";

    template <typename T>
    std::istream &into(T &v) const {
        try {
            __yaml_cpp::Node val;
            try {
                val = __yaml_cpp::Load(stream);
            } catch (std::exception &e) {
                throw error(e.what());
            }
            DESERIALIZE(T){val}.into(v);
        } catch (error &err) {
            if (!filename.empty())
                err.path = std::string(filename);
            throw;
        }
        return stream;
    }

    template <typename T>
    std::istream &operator>>(T &v) const {
        return into(v);
    }
};

template <typename T>
void cpx::yaml::jbeder_yaml::parse(const std::string &str, T &val) {
    Parse<std::string>{str}.into(val, false);
}

template <typename T>
void cpx::yaml::jbeder_yaml::parse(std::istream &stream, T &val, const std::string &filename) {
    Parse<std::istream>{stream}.into(val, filename);
}

template <typename T>
void cpx::yaml::jbeder_yaml::parse_from_file(const std::string &path, T &val) {
    Parse<std::string>{path}.into(val, true);
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T> cpx::yaml::jbeder_yaml::parse(const std::string &str) {
    T val = {};
    Parse<std::string>{str}.into(val, false);
    return val;
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T>
cpx::yaml::jbeder_yaml::parse(std::istream &stream, const std::string &filename) {
    T val = {};
    Parse<std::istream>{stream}.into(val, filename);
    return val;
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T> cpx::yaml::jbeder_yaml::parse_from_file(const std::string &path) {
    T val = {};
    Parse<std::string>{path}.into(val, true);
    return val;
}

template <typename T>
[[nodiscard]]
std::string cpx::yaml::jbeder_yaml::dump(const T &val) {
    return Dump<std::string>{}.from(val);
}

template <typename T>
void cpx::yaml::jbeder_yaml::dump(std::ostream &os, const T &val) {
    Dump<std::ostream>{os}.from(val);
}

namespace cpx::yaml::jbeder_yaml {
    CPX_EXPORT constexpr struct IO {
        friend Dump<std::ostream> operator<<(std::ostream &os, const IO &) {
            return {os};
        }

        friend Parse<std::istream> operator>>(std::istream &is, const IO &) {
            return {is};
        }
    } io{};
} // namespace cpx::yaml::jbeder_yaml

#undef SERIALIZE
#undef DESERIALIZE
#undef SERIALIZABLE
#undef DESERIALIZABLE
#undef DUMP
#undef PARSE
#endif
