#ifndef CPX_CLI_CLI11_H
#define CPX_CLI_CLI11_H

#include <cpx/cli/cli.h>
#include <cpx/serde/deserialize.h>
#include <cpx/serde/error.h>
#include <cpx/extend.h>
#include <cpx/reflect_builtin.h>
#include <variant>

#include <CLI/CLI.hpp>

namespace cpx::cli::cli11 {
    CPX_EXPORT using Parse = cpx::serde::Parse<CLI::App, std::pair<int, char **>>;

    CPX_EXPORT template <typename To>
    using Deserialize = cpx::serde::Deserialize<CLI::App, To>;

    CPX_EXPORT template <typename To>
    using is_deserializable = cpx::serde::is_deserializable<CLI::App, To>;

    CPX_EXPORT template <typename T>
    void parse(const std::string &app_desc, int argc, char **argv, T &v);

    CPX_EXPORT template <typename T>
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(const std::string &app_desc, int argc, char **argv);

    CPX_EXPORT template <typename T>
    std::vector<std::string> parse_with_subcommands(const std::string &app_desc, int argc, char **argv, T &v);

    CPX_EXPORT template <typename T>
    std::enable_if_t<std::is_default_constructible_v<T>, std::pair<T, std::vector<std::string>>>
    parse_with_subcommands(const std::string &app_desc, int argc, char **argv);

    CPX_EXPORT template <typename T>
    std::string help(const std::string &app_desc, int argc, char **argv, T &v);
} // namespace cpx::cli::cli11

namespace cpx {
    CPX_EXPORT namespace cli11 = cpx::cli::cli11;
}

namespace cpx::cli::cli11::detail {
    inline std::string generate_option_name(const cpx::TagInfo &ti, bool subcommand = false) {
        if (ti.positional || subcommand)
            return std::string(ti.key);
        else if (ti.short_.empty())
            return "--" + std::string(ti.key);
        else
            return "-" + std::string(ti.short_) + ",--" + std::string(ti.key);
    }

    template <typename T>
    struct is_primitive_type
        : std::bool_constant<(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) && !std::is_same_v<T, bool>> {};

    template <typename T, typename Enable = void>
    struct is_string_reflect : std::false_type {};

    template <typename T>
    struct is_string_reflect<T, std::enable_if_t<cpx::cli::has_reflect_v<T>>>
        : std::bool_constant<
              ( //
                  std::is_same_v<typename cpx::cli::reflect_traits<T>::type, std::string> &&
                  ( //
                      std::is_same_v<typename cpx::cli::reflect_traits<T>::const_type, std::string> ||
                      std::is_same_v<typename cpx::cli::reflect_traits<T>::const_type, std::string_view>
                  )
              ) ||
              ( //
                  cpx::cli::reflect_traits<T>::has_from_str && cpx::cli::reflect_traits<T>::has_to_str
              )
          > {};

    template <typename T, typename Enable = void>
    struct is_tuple_reflect : std::false_type {};

    template <typename T>
    struct is_tuple_reflect<T, std::enable_if_t<cpx::cli::has_reflect_v<T>>>
        : std::bool_constant<cpx::is_tuple_v<typename cpx::cli::reflect_traits<T>::type>> {};

    template <typename T>
    T from_string(std::string_view str) {
        T val = {};
        if constexpr (cpx::cli::cli11::detail::is_primitive_type<T>::value) {
            bool ok = CLI::detail::lexical_cast(str, val);
            if (!ok)
                throw std::runtime_error("lexical_cast failed");
        } else {
            if constexpr (cpx::cli::reflect_traits<T>::has_from_str) {
                cpx::cli::reflect_traits<T>::from_str(val, str);
            } else {
                decltype(auto) proxy = cpx::cli::reflect_traits<T>::of(val);
                (std::string &)proxy = std::string(str);
            }
        }
        return val;
    }

    template <typename T>
    void set_default(const T &val, CLI::Option *opt) {
        if constexpr (cpx::cli::cli11::detail::is_primitive_type<T>::value) {
            opt->default_val(val);
        } else {
            if constexpr (cpx::cli::reflect_traits<T>::has_to_str) {
                std::string str;
                cpx::cli::reflect_traits<T>::to_str(val, str);
                opt->default_str(str);
            } else {
                decltype(auto) str = cpx::cli::reflect_traits<T>::of(val);
                opt->default_str(std::string(str));
            }
        }
    }

    template <typename T>
    void set_default_vec(const std::vector<T> &val, CLI::Option *opt) {
        if constexpr (cpx::cli::cli11::detail::is_primitive_type<T>::value) {
            opt->default_val(val);
        } else {
            std::vector<std::string> vec(val.size());
            if constexpr (cpx::cli::reflect_traits<T>::has_to_str) {
                for (size_t i = 0; i < val.size(); ++i) {
                    cpx::cli::reflect_traits<T>::to_str(val[i], vec[i]);
                }
                opt->default_val(vec);
            } else {
                for (size_t i = 0; i < val.size(); ++i) {
                    decltype(auto) str = cpx::cli::reflect_traits<T>::of(val[i]);
                    vec[i]             = (std::string)str;
                }
                opt->default_val(vec);
            }
        }
    }
} // namespace cpx::cli::cli11::detail

template <>
struct cpx::serde::Parse<CLI::App, std::pair<int, char **>> {
    std::string               app_desc;
    int                       argc;
    mutable char            **argv;
    std::vector<std::string> *parsed_subcommands = nullptr;

    template <typename T>
    void into(T &tpl) const {
        auto          app = CLI::App(app_desc, argv[0]);
        const TagInfo ti  = {};

        Deserialize<CLI::App, T>{app, ti}.into(tpl);

        argv = app.ensure_utf8(argv);
#ifdef _WIN32
        app.allow_windows_style_options();
#endif
        try {
            app.parse(argc, argv);
        } catch (const CLI::ParseError &e) {
            ::exit(app.exit(e));
        };

        if (parsed_subcommands)
            for (auto *sub : app.get_subcommands())
                parsed_subcommands->push_back(sub->get_name());
    }

    template <typename T>
    std::string help(T &v) const {
        auto       app     = CLI::App(app_desc, argv[0]);
        const bool is_root = true;

        Deserialize<CLI::App, T> d(app, is_root);
        d.into(v);

        return app.help();
    }
};

// bool
template <>
struct cpx::serde::Deserialize<CLI::App, bool> {
    CLI::App      &app;
    const TagInfo &ti;

    CLI::Option *into(bool &v) const {
        return this->app.add_flag(cpx::cli::cli11::detail::generate_option_name(ti), v, std::string(ti.help));
    }
};

// optional bool
template <>
struct cpx::serde::Deserialize<CLI::App, std::optional<bool>> {
    CLI::App      &app;
    const TagInfo &ti;

    CLI::Option *into(std::optional<bool> &v) const {
        auto name = cpx::cli::cli11::detail::generate_option_name(ti);
        name += ",!--no-" + std::string(ti.key.empty() ? ti.short_ : ti.key);
        return this->app.add_flag(name, v, std::string(ti.help));
    }
};

// primitive types
template <typename T>
struct cpx::serde::Deserialize<CLI::App, T, std::enable_if_t<cpx::cli::cli11::detail::is_primitive_type<T>::value>> {
    CLI::App      &app;
    const TagInfo &ti;

    CLI::Option *into(T &v) const {
        CLI::Option *opt = this->app.add_option(cpx::cli::cli11::detail::generate_option_name(ti), v, std::string(ti.help));
        if (!ti.env.empty())
            opt->envname(std::string(ti.env));
        if (!ti.skipmissing)
            opt->required();
        else
            opt->default_val(v);
        return opt;
    }
};

// vector of primitive
template <typename T>
struct cpx::serde::Deserialize<CLI::App, std::vector<T>, std::enable_if_t<cpx::cli::cli11::detail::is_primitive_type<T>::value>> {
    CLI::App      &app;
    const TagInfo &ti;

    CLI::Option *into(std::vector<T> &v) const {
        CLI::Option *opt = this->app.add_option(cpx::cli::cli11::detail::generate_option_name(ti), v, std::string(ti.help));
        if (!ti.env.empty())
            opt->envname(std::string(ti.env));
        if (!v.empty())
            opt->default_val(v);
        return opt;
    }
};

// optional primitive
template <typename T>
struct cpx::serde::
    Deserialize<CLI::App, std::optional<T>, std::enable_if_t<cpx::cli::cli11::detail::is_primitive_type<T>::value>> {
    CLI::App      &app;
    const TagInfo &ti;

    CLI::Option *into(std::optional<T> &v) const {
        CLI::Option *opt = this->app.template add_option<std::optional<T>, T>(
            cpx::cli::cli11::detail::generate_option_name(ti), v, std::string(ti.help)
        );
        if (!ti.env.empty())
            opt->envname(std::string(ti.env));
        if (v.has_value())
            opt->default_val(*v);
        return opt;
    }
};

// variant primitive
template <typename... T>
struct cpx::serde::Deserialize<
    CLI::App,
    std::variant<T...>,
    std::enable_if_t<(
        (cpx::cli::cli11::detail::is_primitive_type<T>::value || cpx::cli::cli11::detail::is_string_reflect<T>::value) && ...
    )>
> {
    CLI::App      &app;
    const TagInfo &ti;

    CLI::Option *into(std::variant<T...> &v) const {
        CLI::Option *opt = this->app.template add_option_function<std::string>(
            cpx::cli::cli11::detail::generate_option_name(ti),
            [&v, this](const std::string &str) {
                bool done = false;
                (
                    [&]() {
                        if (!done) {
                            try {
                                v    = cpx::cli::cli11::detail::from_string<T>(str);
                                done = true;
                            } catch (std::exception &e) {
                                std::ignore = e;
                            }
                        }
                    }(),
                    ...);
                if (!done)
                    throw type_mismatch_error("variant", "unknown"); // TODO
            },
            std::string(ti.help)
        );

        if (!ti.env.empty())
            opt->envname(std::string(ti.env));
        if (!ti.skipmissing)
            opt->required();
        else
            std::visit([opt](const auto &val) { cpx::cli::cli11::detail::set_default(val, opt); }, v);
        return opt;
    }
};

// tuple
template <typename... Ts>
struct cpx::serde::Deserialize<CLI::App, std::tuple<Ts...>> {
    CLI::App      &app;
    const TagInfo &ti;

    CLI::Option *into(std::tuple<Ts...> &tpl) const {
        return into(tpl, nullptr);
    }

    CLI::Option *into(std::tuple<Ts...> &tpl, std::function<void()> cb) const {
        CLI::App *sub = nullptr;
        if (!ti.key.empty()) {
            sub = app.add_subcommand(cpx::cli::cli11::detail::generate_option_name(ti, true), std::string(ti.help));
            if (cb)
                sub->callback(std::move(cb));
        }
        CLI::App &app = sub ? *sub : this->app;

        std::unordered_map<std::string, CLI::Option_group *> oneof_groups;
        cpx::tuple_for_each(cpx::flatten(tpl), [&app, &oneof_groups](auto &item, size_t) {
            const cpx::TagInfo &t         = cpx::cli::get_tag_info(item);
            auto               &v         = cpx::detail::get_underlying_value(item);
            using T                       = std::decay_t<decltype(v)>;
            constexpr bool deserializable = cpx::serde::is_deserializable_v<CLI::App, T>;

            if (!deserializable || t.key == "")
                return;

            if constexpr (deserializable) {
                CLI::Option *opt = Deserialize<CLI::App, T>{app, t}.into(v);
                if (opt && !t.oneof.empty()) {
                    CLI::Option_group *group = get_or_create_group(app, oneof_groups, t.oneof);
                    group->add_option(opt);
                }
            }
        });

        return nullptr;
    }

    static CLI::Option_group *get_or_create_group(
        CLI::App &app, std::unordered_map<std::string, CLI::Option_group *> &oneof_groups, std::string_view name
    ) {
        auto it = oneof_groups.find(std::string(name));
        if (it != oneof_groups.end())
            return it->second;

        auto *g = app.add_option_group(std::string(name));
        g->require_option(1, 1);

        oneof_groups.emplace(name, g);
        return g;
    }
};

// reflect to tuple
template <typename T>
struct cpx::serde::Deserialize<CLI::App, T, std::enable_if_t<cpx::cli11::detail::is_tuple_reflect<T>::value>> {
    CLI::App      &app;
    const TagInfo &ti;

    CLI::Option *into(T &v) const {
        decltype(auto) r = cpx::cli::reflect_traits<T>::of(v);
        return Deserialize<CLI::App, typename cpx::cli::reflect_traits<T>::type>{app, ti}.into(r);
    }
};

// optional reflect to tuple
template <typename T>
struct cpx::serde::Deserialize<CLI::App, std::optional<T>, std::enable_if_t<cpx::cli11::detail::is_tuple_reflect<T>::value>> {
    CLI::App      &app;
    const TagInfo &ti;

    CLI::Option *into(std::optional<T> &v) const {
        if (v.has_value())
            throw error("subcommand optional type must be empty");

        auto val = std::make_shared<T>();

        decltype(auto) r = cpx::cli::reflect_traits<T>::of(*val);
        return Deserialize<CLI::App, typename cpx::cli::reflect_traits<T>::type>{app, ti}.into(r, [val, &v]() { v = *val; });
    }
};

// reflect to string
template <typename T>
struct cpx::serde::Deserialize<CLI::App, T, std::enable_if_t<cpx::cli11::detail::is_string_reflect<T>::value>> {
    CLI::App      &app;
    const TagInfo &ti;

    CLI::Option *into(T &v) const {
        CLI::Option *opt = this->app.template add_option_function<std::string>(
            cpx::cli::cli11::detail::generate_option_name(ti),
            [&v](const std::string &str) {
                try {
                    v = cpx::cli::cli11::detail::from_string<T>(str);
                } catch (std::exception &e) {
                    throw CLI::ParseError("Failed to parse " + str + ": " + e.what(), 1);
                }
                return true;
            },
            std::string(ti.help)
        );

        if (!ti.env.empty())
            opt->envname(std::string(ti.env));
        if (!ti.skipmissing)
            opt->required();
        else
            cpx::cli::cli11::detail::set_default(v, opt);
        return opt;
    }
};

// optional reflect to string
template <typename T>
struct cpx::serde::Deserialize<CLI::App, std::optional<T>, std::enable_if_t<cpx::cli11::detail::is_string_reflect<T>::value>> {
    CLI::App      &app;
    const TagInfo &ti;

    CLI::Option *into(std::optional<T> &v) const {
        CLI::Option *opt = this->app.template add_option_function<std::string>(
            cpx::cli::cli11::detail::generate_option_name(ti),
            [&v](const std::string &str) {
                v = T{};
                try {
                    v = cpx::cli::cli11::detail::from_string<T>(str);
                } catch (std::exception &e) {
                    throw CLI::ParseError("Failed to parse " + str + ": " + e.what(), 1);
                }
                return true;
            },
            std::string(ti.help)
        );

        if (!ti.env.empty())
            opt->envname(std::string(ti.env));
        if (v.has_value())
            cpx::cli::cli11::detail::set_default(*v, opt);
        return opt;
    }
};

// vector reflect to string
template <typename T>
struct cpx::serde::Deserialize<CLI::App, std::vector<T>, std::enable_if_t<cpx::cli::cli11::detail::is_string_reflect<T>::value>> {
    CLI::App      &app;
    const TagInfo &ti;

    CLI::Option *into(std::vector<T> &v) const {
        CLI::Option *opt = this->app.template add_option_function<std::vector<std::string>>(
            cpx::cli::cli11::detail::generate_option_name(ti),
            [&v](const std::vector<std::string> &strs) {
                v.resize(strs.size());
                for (size_t i = 0; i < strs.size(); i++) {
                    auto &dest = v[i];
                    auto &src  = strs[i];
                    try {
                        dest = cpx::cli::cli11::detail::from_string<T>(src);
                    } catch (std::exception &e) {
                        throw CLI::ParseError("Failed to parse " + src + ": " + e.what(), 1);
                    }
                }
                return true;
            },
            std::string(ti.help)
        );

        if (!ti.env.empty())
            opt->envname(std::string(ti.env));
        if (!v.empty())
            cpx::cli::cli11::detail::set_default_vec(v, opt);
        return opt;
    }
};

template <typename T>
void cpx::cli::cli11::parse(const std::string &app_desc, int argc, char **argv, T &v) {
    Parse{app_desc, argc, argv}.into(v);
}

template <typename T>
std::enable_if_t<std::is_default_constructible_v<T>, T>
cpx::cli::cli11::parse(const std::string &app_desc, int argc, char **argv) {
    T v = {};
    Parse{app_desc, argc, argv}.into(v);
    return v;
}

template <typename T>
std::vector<std::string> cpx::cli::cli11::parse_with_subcommands(const std::string &app_desc, int argc, char **argv, T &v) {
    std::vector<std::string> parsed_subcommands;
    Parse{app_desc, argc, argv, &parsed_subcommands}.into(v);
    return parsed_subcommands;
}

template <typename T>
std::enable_if_t<std::is_default_constructible_v<T>, std::pair<T, std::vector<std::string>>>
cpx::cli::cli11::parse_with_subcommands(const std::string &app_desc, int argc, char **argv) {
    T                        v = {};
    std::vector<std::string> parsed_subcommands;
    Parse{app_desc, argc, argv, &parsed_subcommands}.into(v);
    return std::make_pair(std::move(v), std::move(parsed_subcommands));
}

template <typename T>
std::string cpx::cli::cli11::help(const std::string &app_desc, int argc, char **argv, T &v) {
    return Parse{app_desc, argc, argv}.help(v);
}
#endif
