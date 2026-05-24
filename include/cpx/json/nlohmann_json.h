#ifndef CPX_JSON_NLOHMANN_JSON_H
#define CPX_JSON_NLOHMANN_JSON_H

#include <cpx/json/json.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/reflect.h>
#include <cpx/extend.h>
#include <variant>

#ifndef INCLUDE_NLOHMANN_JSON_HPP_
#    include <nlohmann/json.hpp>
#endif

namespace cpx::json::nlohmann_json {
    template <typename T>
    struct Reflect : std::false_type {
        using const_type = type;
    };

    template <typename T>
    struct has_reflect
        : std::bool_constant<(Reflect<T>::value || cpx::json::has_reflect_v<T>) && !std::is_same_v<T, nlohmann::json::value_t>> {
    };

    template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    template <typename T>
    using reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::type, cpx::json::reflect_t<T>>;

    template <typename T>
    using const_reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::const_type, cpx::json::const_reflect_t<T>>;

    template <typename T>
    constexpr decltype(auto) reflect_of(T &&v) {
        if constexpr (Reflect<std::decay_t<T>>::value)
            return Reflect<std::decay_t<T>>::of(std::forward<T>(v));
        else
            return cpx::json::reflect_of(std::forward<T>(v));
    }


    template <typename From>
    using Serialize = cpx::serde::Serialize<nlohmann::json, From>;

    template <typename To>
    using Deserialize = cpx::serde::Deserialize<nlohmann::json, To>;

    template <typename To>
    using Dump = cpx::serde::Dump<nlohmann::json, To>;

    template <typename From = std::string>
    using Parse = ::cpx::serde::Parse<nlohmann::json, From>;

    template <typename T>
    std::string dump(const T &val, int indent = -1, char indent_char = ' ', bool ensure_ascii = false);

    template <typename T>
    void dump(std::ostream &, const T &val, int indent = -1, char indent_char = ' ', bool ensure_ascii = false);

    template <typename T>
    T parse(const std::string &str, bool ignore_comments = false);

    template <typename T>
    T parse(std::istream &stream, bool ignore_comments = false);

    template <typename T>
    T parse(FILE *pfile, bool ignore_comments = false);

    template <typename T>
    void parse(const std::string &str, T &val, bool ignore_comments = false);

    template <typename T>
    void parse(std::istream &stream, T &val, bool ignore_comments = false);

    template <typename T>
    void parse(FILE *pfile, T &val, bool ignore_comments = false);
} // namespace cpx::json::nlohmann_json

// optional
template <typename T>
struct nlohmann::adl_serializer<
    std::optional<T>,
    std::enable_if_t<
        std::is_convertible_v<T, nlohmann::json>,
        std::void_t<decltype(std::declval<const nlohmann::json &>().get<T>())>>> {

    static void to_json(json &j, const std::optional<T> &opt) {
        if (opt.has_value())
            j = *opt;
        else
            j = nullptr;
    }

    static void from_json(const json &j, std::optional<T> &opt) {
        if (j.is_null())
            opt.reset();
        else
            opt = j.get<T>();
    }
};

// variant
template <typename... T>
struct nlohmann::adl_serializer<
    std::variant<T...>,
    std::enable_if_t<((std::is_convertible_v<T, nlohmann::json> && cpx::serde::is_deserializable_v<nlohmann::json, T>) && ...)>> {

    static void to_json(json &j, const std::variant<T...> &v) {
        std::visit([&](const auto &var) { j = var; }, v);
    }

    static void from_json(const json &j, std::variant<T...> &v) {
        bool        done = false;
        std::string type_names;
        (
            [&]() {
                try {
                    if (!done) {
                        auto element = T{};
                        cpx::serde::Deserialize<nlohmann::json, T>{j}.into(element);
                        v    = std::move(element);
                        done = true;
                    }
                } catch (cpx::serde::type_mismatch_error &e) {
                    type_names += e.expected_type + '|';
                }
            }(),
            ...);
        if (!done) {
            type_names.pop_back();
            throw cpx::serde::type_mismatch_error(type_names, j.type_name());
        }
    }
};

// tuple
template <typename... Ts>
struct nlohmann::adl_serializer<std::tuple<Ts...>> {
    static void to_json(json &j, const std::tuple<Ts...> &tpl) {
        auto flattened        = flatten(tpl);
        using Tpl             = decltype(flattened);
        constexpr bool is_obj = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;

        j = is_obj ? nlohmann::json::object() : nlohmann::json::array();

        size_t idx = 0;
        cpx::tuple_for_each(flattened, [&](const auto &item, const size_t) {
            const cpx::TagInfo &t       = cpx::json::get_tag_info(item);
            const auto         &v       = cpx::detail::get_underlying_value(item);
            using T                     = std::decay_t<decltype(v)>;
            constexpr bool serializable = cpx::serde::is_serializable_v<nlohmann::json, T>;

            if (!serializable || (is_obj && t.key == ""))
                return;

            size_t i = idx++;
            if (t.omitempty && cpx::detail::is_empty_value(v))
                return;

            auto &val = is_obj ? j[t.key] : j[i];
            try {
                if (t.noserde)
                    if constexpr (std::is_same_v<T, std::string>)
                        val = nlohmann::json::parse(v);
                    else
                        throw cpx::serde::error("field with tag `noserde` can only be serialized from std::string");
                else {
                    if constexpr (serializable)
                        try {
                            val = v;
                        } catch (nlohmann::json::exception &e) {
                            throw cpx::serde::error(e.what());
                        }
                }
            } catch (cpx::serde::error &e) {
                if (is_obj)
                    e.add_context(t.key);
                else
                    e.add_context(i);
                throw;
            }
        });
    }

    static void from_json(const json &j, std::tuple<Ts...> &tpl) {
        auto flattened        = flatten(tpl);
        using Tpl             = decltype(flattened);
        constexpr bool is_obj = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;

        if (is_obj && !j.is_object())
            throw cpx::serde::type_mismatch_error("object", j.type_name());
        if (!is_obj && !j.is_array())
            throw cpx::serde::type_mismatch_error("array", j.type_name());

        size_t pos = 0;
        cpx::tuple_for_each(flattened, [&](auto &item, const size_t) {
            const cpx::TagInfo &t         = cpx::json::get_tag_info(item);
            auto               &v         = cpx::detail::get_underlying_value(item);
            using T                       = std::decay_t<decltype(v)>;
            constexpr bool deserializable = cpx::serde::is_deserializable_v<nlohmann::json, T>;

            if (!deserializable || (is_obj && t.key == ""))
                return;

            size_t                idx = pos++;
            const nlohmann::json *ptr = nullptr;
            try {
                try {
                    ptr = is_obj ? &j.at(t.key) : &j.at(idx);
                } catch (nlohmann::json::exception &e) {
                    if (t.skipmissing)
                        return;
                    else
                        throw cpx::serde::error(e.what());
                }
                if (t.noserde)
                    if constexpr (std::is_same_v<T, std::string>)
                        v = ptr->dump();
                    else
                        throw cpx::serde::error("field with tag `noserde` can only be deserialized into std::string");
                else {
                    if constexpr (deserializable)
                        try {
                            ptr->get_to(v);
                        } catch (nlohmann::json::exception &e) {
                            throw cpx::serde::error(e.what());
                        }
                }
            } catch (cpx::serde::error &e) {
                if (is_obj)
                    e.add_context(t.key);
                else
                    e.add_context(idx);
                throw;
            }
        });
    }
};

// reflection
template <typename T>
struct nlohmann::adl_serializer<T, std::enable_if_t<cpx::json::nlohmann_json::has_reflect_v<T>>> {
    static void to_json(nlohmann::json &j, const T &v) {
        j = cpx::serde::Serialize<nlohmann::json, cpx::json::nlohmann_json::const_reflect_t<T>>{}.from(
            cpx::json::nlohmann_json::reflect_of(v)
        );
    }

    static void from_json(const nlohmann::json &j, T &v) {
        decltype(auto) proxy = cpx::json::nlohmann_json::reflect_of(v);
        cpx::serde::Deserialize<nlohmann::json, cpx::json::nlohmann_json::reflect_t<T>>{j}.into(proxy);
    }
};

template <typename T>
struct cpx::serde::Serialize<nlohmann::json, T, std::enable_if_t<std::is_convertible_v<T, nlohmann::json>>> {
    nlohmann::json from(const T &v) const {
        try {
            return v;
        } catch (nlohmann::json::exception &e) {
            throw error(e.what());
        }
    }
};

template <typename T>
struct cpx::serde::
    Deserialize<nlohmann::json, T, std::void_t<decltype(std::declval<const nlohmann::json &>().get_to(std::declval<T &>()))>> {
    const nlohmann::json &j;

    void into(T &v) const {
        try {
            j.get_to(v);
        } catch (nlohmann::json::type_error &e) {
            auto [expected, got] = extract_types(e.what());
            throw type_mismatch_error(expected, got);
        } catch (nlohmann::json::exception &e) {
            throw error(e.what());
        }
    }

    static std::pair<std::string, std::string> extract_types(std::string_view message) {
        std::string expected_type;
        std::string got_type;

        std::string_view prefix1 = "type must be ";
        std::string_view prefix2 = ", but is ";

        auto pos1 = message.find(prefix1);
        auto pos2 = message.find(prefix2);
        if (pos1 == std::string_view::npos || pos2 == std::string_view::npos)
            return {};

        expected_type = message.substr(pos1 + prefix1.size(), pos2 - (pos1 + prefix1.size()));
        got_type      = message.substr(pos2 + prefix2.size());
        return {expected_type, got_type};
    }
};

template <>
struct cpx::serde::Parse<nlohmann::json, std::string> {
    const std::string &str;
    bool               ignore_comments = false;

    template <typename T>
    void into(T &val) const {
        try {
            auto j = nlohmann::json::parse(str, nullptr, true, ignore_comments);
            Deserialize<nlohmann::json, T>{j}.into(val);
        } catch (nlohmann::json::exception &e) {
            throw error(e.what());
        }
    }
};

template <>
struct cpx::serde::Parse<nlohmann::json, std::istream> {
    std::istream &stream;
    bool          ignore_comments = false;

    template <typename T>
    std::istream &into(T &val) const {
        try {
            auto j = nlohmann::json::parse(stream, nullptr, true, ignore_comments);
            Deserialize<nlohmann::json, T>{j}.into(val);
        } catch (nlohmann::json::exception &e) {
            throw error(e.what());
        }
    }

    template <typename T>
    std::istream &operator>>(T &val) const {
        return into(val);
    }
};

template <>
struct cpx::serde::Parse<nlohmann::json, std::FILE *> {
    std::FILE *file;
    bool       ignore_comments = false;

    template <typename T>
    void into(T &val) const {
        try {
            auto j = nlohmann::json::parse(file, nullptr, true, ignore_comments);
            Deserialize<nlohmann::json, T>{j}.into(val);
        } catch (nlohmann::json::exception &e) {
            throw error(e.what());
        }
    }
};

template <>
struct cpx::serde::Dump<nlohmann::json, std::ostream> {
    std::ostream &os;
    int           indent       = -1;
    char          indent_char  = ' ';
    bool          ensure_ascii = false;

    template <typename T>
    void from(const T &val) const {
        try {
            os << Serialize<nlohmann::json, T>{}.from(val);
        } catch (nlohmann::json::exception &e) {
            throw error(e.what());
        }
    }

    template <typename T>
    std::ostream &operator<<(T &val) const {
        return from(val);
    }
};

template <>
struct cpx::serde::Dump<nlohmann::json, std::string> {
    int  indent       = -1;
    char indent_char  = ' ';
    bool ensure_ascii = false;

    template <typename T>
    std::string from(const T &val) const {
        try {
            return Serialize<nlohmann::json, T>{}.from(val).dump(indent, indent_char, ensure_ascii);
        } catch (nlohmann::json::exception &e) {
            throw error(e.what());
        }
    }
};

template <typename T>
std::string cpx::json::nlohmann_json::dump(const T &val, int indent, char indent_char, bool ensure_ascii) {
    return Dump<std::string>{indent, indent_char, ensure_ascii}.from(val);
}

template <typename T>
void cpx::json::nlohmann_json::dump(std::ostream &os, const T &val, int indent, char indent_char, bool ensure_ascii) {
    Dump<std::ostream>{os, indent, indent_char, ensure_ascii}.from(val);
}

template <typename T>
T cpx::json::nlohmann_json::parse(const std::string &str, bool ignore_comments) {
    T val = {};
    Parse<std::string>{str, ignore_comments}.into(val);
    return val;
}

template <typename T>
T cpx::json::nlohmann_json::parse(std::istream &stream, bool ignore_comments) {
    T val = {};
    Parse<std::istream>{stream, ignore_comments}.into(val);
    return val;
}

template <typename T>
T cpx::json::nlohmann_json::parse(FILE *pfile, bool ignore_comments) {
    T val = {};
    Parse<FILE *>{pfile, ignore_comments}.into(val);
    return val;
}

template <typename T>
void cpx::json::nlohmann_json::parse(const std::string &str, T &val, bool ignore_comments) {
    Parse<std::string>{str, ignore_comments}.into(val);
}

template <typename T>
void cpx::json::nlohmann_json::parse(std::istream &stream, T &val, bool ignore_comments) {
    Parse<std::istream>{stream, ignore_comments}.into(val);
}

template <typename T>
void cpx::json::nlohmann_json::parse(FILE *pfile, T &val, bool ignore_comments) {
    Parse<FILE *>{pfile, ignore_comments}.into(val);
}

namespace cpx::json::nlohmann_json {
    constexpr struct IO {
        int  _indent          = -1;
        char _indent_char     = ' ';
        bool _ensure_ascii    = true;
        bool _ignore_comments = false; // for input

        constexpr IO indent(int val) const {
            IO self      = *this;
            self._indent = val;
            return self;
        }

        constexpr IO indent_char(char val) const {
            IO self           = *this;
            self._indent_char = val;
            return self;
        }

        constexpr IO ensure_ascii(bool val) const {
            IO self            = *this;
            self._ensure_ascii = val;
            return self;
        }

        constexpr IO ignore_comments(bool val = true) const {
            IO self               = *this;
            self._ignore_comments = val;
            return self;
        }

        friend cpx::serde::Dump<nlohmann::json, std::ostream> operator<<(std::ostream &os, const IO &io) {
            return {os, io._indent, io._indent_char, io._ensure_ascii};
        }

        friend cpx::serde::Parse<nlohmann::json, std::istream> operator>>(std::istream &is, const IO &io) {
            return {is, io._ignore_comments};
        }
    } io{};
} // namespace cpx::json::nlohmann_json
#endif
