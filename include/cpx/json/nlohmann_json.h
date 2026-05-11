#ifndef CPX_JSON_NLOHMANN_JSON_H
#define CPX_JSON_NLOHMANN_JSON_H

#include <cpx/json/json.h>
#include <cpx/serde/error.h>
#include <variant>

#ifndef INCLUDE_NLOHMANN_JSON_HPP_
#    include <nlohmann/json.hpp>
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

namespace cpx::json::nlohmann_json {
    template <typename From>
    using Serialize = ::cpx::serde::Serialize<nlohmann::json, From>;

    template <typename To>
    using Deserialize = ::cpx::serde::Deserialize<nlohmann::json, To>;

    template <typename To>
    using Dump = ::cpx::serde::Dump<nlohmann::json, To>;

    template <typename From = std::string>
    using Parse = ::cpx::serde::Parse<nlohmann::json, From>;

    template <typename T>
    std::string dump(const T &val, int indent = -1, char indent_char = ' ', bool ensure_ascii = false);

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

namespace nlohmann {
    // optional
    template <typename T>
    struct adl_serializer<
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

    // tuple
    template <typename... Ts>
    struct adl_serializer<std::tuple<Ts...>> {
        static void to_json(json &j, const std::tuple<Ts...> &tpl) {
            const cpx::TagInfoTuple<sizeof...(Ts)>         ts     = cpx::json::get_tag_info_from_tuple(tpl);
            const std::array<cpx::TagInfo, sizeof...(Ts)> &ti     = ts.ts;
            const bool                                     is_obj = ts.is_obj;

            if (is_obj)
                j = nlohmann::json::object();
            else
                j = nlohmann::json::array();

            cpx::tuple_for_each(tpl, [&](const auto &item, const size_t i) {
                const cpx::TagInfo &t       = ti[i];
                const auto         &v       = cpx::detail::get_underlying_value(item);
                using T                     = std::decay_t<decltype(v)>;
                constexpr bool serializable = cpx::serde::is_serializable_v<nlohmann::json, T>;

                if (!serializable || (is_obj && t.key == "") || (t.omitempty && cpx::detail::is_empty_value(v)))
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
            from_json_tpl(j, tpl);
        }

        static void from_json(const json &j, const std::tuple<Ts...> &tpl) {
            from_json_tpl(j, tpl);
        }

    private:
        template <typename Tpl>
        static void from_json_tpl(const json &j, Tpl &tpl) {
            const cpx::TagInfoTuple<sizeof...(Ts)>         ts     = cpx::json::get_tag_info_from_tuple(tpl);
            const std::array<cpx::TagInfo, sizeof...(Ts)> &ti     = ts.ts;
            const bool                                     is_obj = ts.is_obj;

            if (is_obj && !j.is_object())
                throw cpx::serde::type_mismatch_error("object", j.type_name());
            if (!is_obj && !j.is_array())
                throw cpx::serde::type_mismatch_error("array", j.type_name());

            cpx::tuple_for_each(tpl, [&](auto &item, const size_t i) {
                const cpx::TagInfo &t         = ti[i];
                auto               &v         = cpx::detail::get_underlying_value(item);
                using T                       = std::decay_t<decltype(v)>;
                constexpr bool deserializable = cpx::serde::is_deserializable_v<nlohmann::json, T>;

                if (!deserializable || (is_obj && t.key == ""))
                    return;

                const nlohmann::json *ptr = nullptr;
                try {
                    try {
                        ptr = is_obj ? &j.at(t.key) : &j.at(i);
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
                        e.add_context(i);
                    throw;
                }
            });
        }
    };

    // variant
    template <typename... T>
    struct adl_serializer<
        std::variant<T...>,
        std::enable_if_t<
            ((std::is_convertible_v<T, nlohmann::json> && cpx::serde::is_deserializable_v<nlohmann::json, T>) && ...)>> {

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
                ...
            );
            if (!done) {
                type_names.pop_back();
                throw cpx::serde::type_mismatch_error(type_names, j.type_name());
            }
        }
    };

    template <typename T>
    struct adl_serializer<T, std::enable_if_t<cpx::json::SerializeAs<T>::value && cpx::json::DeserializeAs<T>::value>> {
        using S = cpx::json::SerializeAs<T>;
        using D = cpx::json::DeserializeAs<T>;

        static void to_json(nlohmann::json &j, const T &v) {
            auto hook = S(v);

            j = cpx::serde::Serialize<nlohmann::json, typename S::type>{}.from(hook);
        }

        static void from_json(const nlohmann::json &j, T &v) {
            auto hook = D(v);

            if constexpr (cpx::is_tuple_v<typename D::type>) {
                typename D::type tpl = hook;
                cpx::serde::Deserialize<nlohmann::json, typename D::type>{j}.into(tpl);
            } else {
                cpx::serde::Deserialize<nlohmann::json, typename D::type>{j}.into(hook);
            }
        }
    };

#ifdef BOOST_PFR_HPP
    template <typename S>
    struct adl_serializer<
        S,
        std::enable_if_t<std::is_aggregate_v<S> && !(cpx::json::SerializeAs<S>::value || cpx::json::DeserializeAs<S>::value)>> {
        static void from_json(const json &j, S &v) {
            auto tpl = boost::pfr::structure_tie(v);
            j.get_to(tpl);
        }

        static void to_json(json &j, const S &v) {
            j = boost::pfr::structure_tie(v);
        }
    };
#endif

#ifdef NEARGYE_MAGIC_ENUM_HPP
    template <typename S>
    struct adl_serializer<
        S,
        std::enable_if_t<std::is_enum_v<S> && !(cpx::json::SerializeAs<S>::value || cpx::json::DeserializeAs<S>::value)>> {
        static void from_json(const json &j, S &v) {
            auto str = j.get<std::string>();
            auto e   = magic_enum::enum_cast<S>(str);
            if (!e.has_value()) {
                std::string what = "invalid value `" + str + "`, expected one of {";
                for (std::string_view name : magic_enum::enum_names<S>()) {
                    what += std::string(name) + ",";
                }
                what += "}";
                throw cpx::serde::error(what);
            }
            v = *e;
        }

        static void to_json(json &j, const S &v) {
            j = magic_enum::enum_name(v);
        }
    };
#endif
} // namespace nlohmann

namespace cpx::serde {
    template <typename T>
    struct Serialize<nlohmann::json, T, std::enable_if_t<std::is_convertible_v<T, nlohmann::json>>> {
        nlohmann::json from(const T &v) const {
            try {
                return v;
            } catch (nlohmann::json::exception &e) {
                throw error(e.what());
            }
        }
    };

    template <typename T>
    struct Deserialize<
        nlohmann::json,
        T,
        std::void_t<decltype(std::declval<const nlohmann::json &>().get_to(std::declval<T &>()))>> {
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
    struct Parse<nlohmann::json, std::string> {
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
    struct Parse<nlohmann::json, std::istream> {
        std::istream &stream;
        bool          ignore_comments = false;

        template <typename T>
        void into(T &val) const {
            try {
                auto j = nlohmann::json::parse(stream, nullptr, true, ignore_comments);
                Deserialize<nlohmann::json, T>{j}.into(val);
            } catch (nlohmann::json::exception &e) {
                throw error(e.what());
            }
        }
    };

    template <>
    struct Parse<nlohmann::json, std::FILE *> {
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
    struct Dump<nlohmann::json, std::ostream> {
        int  indent       = -1;
        char indent_char  = ' ';
        bool ensure_ascii = false;

        template <typename T>
        nlohmann::json from(const T &val) const {
            try {
                return Serialize<nlohmann::json, T>{}.from(val);
            } catch (nlohmann::json::exception &e) {
                throw error(e.what());
            }
        }
    };

    template <>
    struct Dump<nlohmann::json, std::string> {
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
} // namespace cpx::serde

namespace cpx::json::nlohmann_json {
    template <typename T>
    std::string dump(const T &val, int indent, char indent_char, bool ensure_ascii) {
        return Dump<std::string>{indent, indent_char, ensure_ascii}.from(val);
    }

    template <typename T>
    nlohmann::json dump_to_stream(const T &val, int indent, char indent_char, bool ensure_ascii) {
        return Dump<std::ostream>{indent, indent_char, ensure_ascii}.from(val);
    }

    template <typename T>
    T parse(const std::string &str, bool ignore_comments) {
        T val = {};
        Parse<std::string>{str, ignore_comments}.into(val);
        return val;
    }

    template <typename T>
    T parse(std::istream &stream, bool ignore_comments) {
        T val = {};
        Parse<std::istream>{stream, ignore_comments}.into(val);
        return val;
    }

    template <typename T>
    T parse(FILE *pfile, bool ignore_comments) {
        T val = {};
        Parse<FILE *>{pfile, ignore_comments}.into(val);
        return val;
    }

    template <typename T>
    void parse(const std::string &str, T &val, bool ignore_comments) {
        Parse<std::string>{str, ignore_comments}.into(val);
    }

    template <typename T>
    void parse(std::istream &stream, T &val, bool ignore_comments) {
        Parse<std::istream>{stream, ignore_comments}.into(val);
    }

    template <typename T>
    void parse(FILE *pfile, T &val, bool ignore_comments) {
        Parse<FILE *>{pfile, ignore_comments}.into(val);
    }
} // namespace cpx::json::nlohmann_json
#endif
