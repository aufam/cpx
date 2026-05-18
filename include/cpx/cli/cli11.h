#ifndef CPX_CLI_CLI11_H
#define CPX_CLI_CLI11_H

#include <cpx/cli/cli.h>
#include <cpx/serde/deserialize.h>
#include <cpx/serde/error.h>
#include <cpx/extend.h>
#include <variant>

#include <CLI/CLI.hpp>

namespace cpx::cli::cli11 {
    using Parse = ::cpx::serde::Parse<CLI::App, std::pair<int, char **>>;

    template <typename To>
    using Deserialize = ::cpx::serde::Deserialize<CLI::App, To>;

    template <typename To>
    using is_deserializable = ::cpx::serde::is_deserializable<CLI::App, To>;

    template <typename T>
    void parse(const std::string &app_desc, int argc, char **argv, T &v);

    template <typename T>
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(const std::string &app_desc, int argc, char **argv);

    template <typename T>
    std::vector<std::string> parse_with_subcommands(const std::string &app_desc, int argc, char **argv, T &v);

    template <typename T>
    std::enable_if_t<std::is_default_constructible_v<T>, std::pair<T, std::vector<std::string>>>
    parse_with_subcommands(const std::string &app_desc, int argc, char **argv);

    template <typename T>
    std::string help(const std::string &app_desc, int argc, char **argv, T &v);
} // namespace cpx::cli::cli11

namespace cpx::cli::cli11::detail {
    inline std::string convert_flag_format(std::string_view input, bool positional, bool subcommand) {
        if (subcommand)
            return std::string(input);
        else if (input.size() <= 1 || input[1] != '|')
            return (positional ? "" : "--") + std::string(input);
        else if (positional)
            return std::string(input.substr(2));
        else
            return "-" + std::string(input.substr(0, 1)) + ",--" + std::string(input.substr(2));
    }

    class DeserializeDispatcher {
    public:
        explicit DeserializeDispatcher(CLI::App &app, bool is_root = false)
            : app(app)
            , is_root(is_root) {}

        CLI::App   &app;
        bool        is_root;
        std::string option_name;
        std::string help_string;
        std::string env;
        bool        positional = false;
        bool        required   = false;
    };

    template <typename T>
    class DeserializeDispatcherFor : public DeserializeDispatcher {
        using DeserializeDispatcher::DeserializeDispatcher;

    public:
        virtual void into(T &v) const = 0;

        DeserializeDispatcherFor<T> &configure(const TagInfo &ti) {
            option_name = cpx::cli::cli11::detail::
                convert_flag_format(ti.key, ti.positional, is_tuple_v<T> || is_tuple_v<cpx::cli::reflect_t<T>>);
            help_string = std::string(ti.help);
            positional  = ti.positional;
            required    = !ti.skipmissing;
            env         = std::string(ti.env);
            return *this;
        }

        DeserializeDispatcherFor<T> &configure(const DeserializeDispatcher &other) {
            is_root     = other.is_root;
            option_name = other.option_name;
            help_string = other.help_string;
            positional  = other.positional;
            required    = other.required;
            env         = other.env;
            return *this;
        }
    };
} // namespace cpx::cli::cli11::detail


namespace cpx::serde {
    template <>
    struct Parse<CLI::App, std::pair<int, char **>> {
        std::string               app_desc;
        int                       argc;
        mutable char            **argv;
        std::vector<std::string> *parsed_subcommands = nullptr;

        template <typename T>
        void into(T &tpl) const {
            auto       app     = CLI::App(app_desc, argv[0]);
            const bool is_root = true;

            Deserialize<CLI::App, T> d(app, is_root);
            d.into(tpl);

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

        template <typename... Ts>
        std::string help(std::tuple<Ts...> &tpl) const {
            auto       app     = CLI::App(app_desc, argv[0]);
            const bool is_root = true;

            Deserialize<CLI::App, std::tuple<Ts...>> d(app, is_root);
            d.into(tpl);

            return app.help();
        }
    };

    template <>
    struct Deserialize<CLI::App, bool> : public cli::cli11::detail::DeserializeDispatcherFor<bool> {
        using cli::cli11::detail::DeserializeDispatcherFor<bool>::DeserializeDispatcherFor;

        void into(bool &v) const override {
            this->app.add_flag(this->option_name, v, this->help_string);
        }
    };

    template <typename T>
    struct Deserialize<
        CLI::App,
        T,
        std::enable_if_t<(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) && !std::is_same_v<T, bool>>>
        : public cli::cli11::detail::DeserializeDispatcherFor<T> {
        using cli::cli11::detail::DeserializeDispatcherFor<T>::DeserializeDispatcherFor;

        void into(T &v) const override {
            CLI::Option *opt = this->app.add_option(this->option_name, v, this->help_string);
            if (!this->env.empty())
                opt->envname(this->env);
            if (this->required)
                opt->required(this->required);
            else
                opt->default_val(v);
        }
    };

    template <typename T>
    struct Deserialize<
        CLI::App,
        std::vector<T>,
        std::enable_if_t<(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) && !std::is_same_v<T, bool>>>
        : public cli::cli11::detail::DeserializeDispatcherFor<std::vector<T>> {
        using cli::cli11::detail::DeserializeDispatcherFor<std::vector<T>>::DeserializeDispatcherFor;

        void into(std::vector<T> &v) const override {
            CLI::Option *opt = this->app.add_option(this->option_name, v, this->help_string);
            if (!this->env.empty())
                opt->envname(this->env);
            if (!v.empty())
                opt->default_val(v);
        }
    };

    template <typename T>
    struct Deserialize<
        CLI::App,
        std::optional<T>,
        std::enable_if_t<(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) && !std::is_same_v<T, bool>>>
        : public cli::cli11::detail::DeserializeDispatcherFor<std::optional<T>> {
        using cli::cli11::detail::DeserializeDispatcherFor<std::optional<T>>::DeserializeDispatcherFor;

        void into(std::optional<T> &v) const override {
            CLI::Option *opt = this->app.template add_option<std::optional<T>, T>(this->option_name, v, this->help_string);
            if (!this->env.empty())
                opt->envname(this->env);
            if (v.has_value())
                opt->default_val(*v);
        }
    };


    template <typename... T>
    struct Deserialize<
        CLI::App,
        std::variant<T...>,
        std::enable_if_t<(((std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) && !std::is_same_v<T, bool>) && ...)>>
        : public cli::cli11::detail::DeserializeDispatcherFor<std::variant<T...>> {
        using cli::cli11::detail::DeserializeDispatcherFor<std::variant<T...>>::DeserializeDispatcherFor;

        void into(std::variant<T...> &v) const override {
            CLI::Option *opt = this->app.template add_option_function<std::string>(
                this->option_name,
                [&v, this](const std::string &str) {
                    bool done = false;
                    (
                        [&]() {
                            if (!done) {
                                auto element = T{};
                                done         = CLI::detail::lexical_cast(str, element);
                                if (done)
                                    v = std::move(element);
                            }
                        }(),
                        ...
                    );
                    if (!done)
                        throw type_mismatch_error("variant", "unknown"); // TODO
                },
                this->help_string
            );

            if (!this->env.empty())
                opt->envname(this->env);
            if (this->required)
                opt->required(this->required);
            else
                std::visit([opt](auto &val) { opt->default_val(val); }, v);
        }
    };

    template <typename... Ts>
    struct Deserialize<CLI::App, std::tuple<Ts...>> : public cli::cli11::detail::DeserializeDispatcherFor<std::tuple<Ts...>> {
        using cli::cli11::detail::DeserializeDispatcherFor<std::tuple<Ts...>>::DeserializeDispatcherFor;

        void into(std::tuple<Ts...> &tpl) const override {
            CLI::App *sub = nullptr;
            if (!this->is_root)
                sub = this->app.add_subcommand(this->option_name, this->help_string);

            tuple_for_each(tpl, [&](auto &item, size_t) {
                const cpx::TagInfo &t         = cpx::cli::get_tag_info(item);
                auto               &v         = detail::get_underlying_value(item);
                using T                       = std::decay_t<decltype(v)>;
                constexpr bool deserializable = cpx::serde::is_deserializable_v<CLI::App, T>;

                if (!deserializable || t.key == "")
                    return;

                if constexpr (deserializable) {
                    Deserialize<CLI::App, T> d(sub ? *sub : this->app);
                    d.configure(t).into(v);
                }
            });
        }
    };

    template <typename T>
    struct Deserialize<CLI::App, T, std::enable_if_t<cpx::cli::has_reflect_v<T> && cpx::is_tuple_v<cpx::cli::reflect_t<T>>>>
        : public cli::cli11::detail::DeserializeDispatcherFor<T> {
        using cli::cli11::detail::DeserializeDispatcherFor<T>::DeserializeDispatcherFor;

        void into(T &v) const override {
            Deserialize<CLI::App, cpx::cli::reflect_t<T>> d(this->app);

            decltype(auto) r = cpx::cli::reflect_of(v);
            d.configure(*this).into(r);
        }
    };

    template <typename T>
    struct Deserialize<
        CLI::App,
        T,
        std::enable_if_t<cpx::cli::has_reflect_v<T> && std::is_same_v<cpx::cli::reflect_t<T>, std::string>>>
        : public cli::cli11::detail::DeserializeDispatcherFor<T> {
        using cli::cli11::detail::DeserializeDispatcherFor<T>::DeserializeDispatcherFor;

        void into(T &v) const override {
            CLI::Option *opt = this->app.template add_option_function<std::string>(
                this->option_name,
                [&v](const std::string &str) {
                    try {
                        decltype(auto) proxy = cpx::cli::reflect_of(v);
                        decltype(auto) p     = (std::string &)proxy;
                        p                    = str;
                    } catch (std::exception &e) {
                        throw CLI::ParseError("Failed to parse " + str + ": " + e.what(), 1);
                    }
                    return true;
                },
                this->help_string
            );

            if (!this->env.empty())
                opt->envname(this->env);
            if (this->required)
                opt->required(this->required);
            else {
                const auto    &cv  = v;
                decltype(auto) str = cpx::cli::reflect_of(cv);
                opt->default_str(std::string(str));
            }
        }
    };

    template <typename T>
    struct Deserialize<
        CLI::App,
        std::optional<T>,
        std::enable_if_t<cpx::cli::has_reflect_v<T> && std::is_same_v<cpx::cli::reflect_t<T>, std::string>>>
        : public cli::cli11::detail::DeserializeDispatcherFor<std::optional<T>> {
        using cli::cli11::detail::DeserializeDispatcherFor<std::optional<T>>::DeserializeDispatcherFor;

        void into(std::optional<T> &v) const override {
            CLI::Option *opt = this->app.template add_option_function<std::string>(
                this->option_name,
                [&v](const std::string &str) {
                    v = T{};
                    try {
                        decltype(auto) proxy = cpx::cli::reflect_of(v);
                        decltype(auto) p     = (std::string &)proxy;
                        p                    = str;
                    } catch (std::exception &e) {
                        throw CLI::ParseError("Failed to parse " + str + ": " + e.what(), 1);
                    }
                    return true;
                },
                this->help_string
            );

            if (!this->env.empty())
                opt->envname(this->env);
            if (v.has_value()) {
                const auto    &cv  = *v;
                decltype(auto) str = cpx::cli::reflect_of(cv);
                opt->default_str(std::string(str));
            }
        }
    };

    template <typename T>
    struct Deserialize<
        CLI::App,
        std::vector<T>,
        std::enable_if_t<cpx::cli::has_reflect_v<T> && std::is_same_v<cpx::cli::reflect_t<T>, std::string>>>
        : public cli::cli11::detail::DeserializeDispatcherFor<std::vector<T>> {
        using cli::cli11::detail::DeserializeDispatcherFor<std::vector<T>>::DeserializeDispatcherFor;

        void into(std::vector<T> &v) const override {
            CLI::Option *opt = this->app.template add_option_function<std::vector<std::string>>(
                this->option_name,
                [&v](const std::vector<std::string> &strs) {
                    v.resize(strs.size());
                    for (size_t i = 0; i < strs.size(); i++) {
                        auto &dest = v[i];
                        auto &src  = strs[i];
                        try {
                            decltype(auto) proxy = cpx::cli::reflect_of(dest);
                            decltype(auto) p     = (std::string &)proxy;
                            p                    = src;
                        } catch (std::exception &e) {
                            throw CLI::ParseError("Failed to parse " + src + ": " + e.what(), 1);
                        }
                    }
                    return true;
                },
                this->help_string
            );

            if (!this->env.empty())
                opt->envname(this->env);
            if (!v.empty()) {
                std::vector<std::string> enum_names;
                for (const auto &cv : v) {
                    decltype(auto) str = cpx::cli::reflect_of(cv);
                    enum_names.push_back(std::string(str));
                }
                opt->default_val(enum_names);
            }
        }
    };
} // namespace cpx::serde

namespace cpx::cli::cli11 {
    template <typename T>
    void parse(const std::string &app_desc, int argc, char **argv, T &v) {
        Parse{app_desc, argc, argv}.into(v);
    }

    template <typename T>
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(const std::string &app_desc, int argc, char **argv) {
        T v;
        Parse{app_desc, argc, argv}.into(v);
        return v;
    }

    template <typename T>
    std::vector<std::string> parse_with_subcommands(const std::string &app_desc, int argc, char **argv, T &v) {
        std::vector<std::string> parsed_subcommands;
        Parse{app_desc, argc, argv, &parsed_subcommands}.into(v);
        return parsed_subcommands;
    }

    template <typename T>
    std::enable_if_t<std::is_default_constructible_v<T>, std::pair<T, std::vector<std::string>>>
    parse_with_subcommands(const std::string &app_desc, int argc, char **argv) {
        T                        v;
        std::vector<std::string> parsed_subcommands;
        Parse{app_desc, argc, argv, &parsed_subcommands}.into(v);
        return std::make_pair(std::move(v), std::move(parsed_subcommands));
    }

    template <typename T>
    std::string help(const std::string &app_desc, int argc, char **argv, T &v) {
        return Parse{app_desc, argc, argv}.help(v);
    }
} // namespace cpx::cli::cli11
#endif
