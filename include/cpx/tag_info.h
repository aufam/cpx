#ifndef CPX_TAG_INFO_H
#define CPX_TAG_INFO_H

#include <cpx/tag.h>
#include <cpx/optional.h>
#include <cpx/tuple.h>

#ifndef CPX_EXPORT
#    define CPX_EXPORT
#endif

namespace cpx {
    CPX_EXPORT struct TagInfo {
        std::string_view key = "";

        // for cli
        std::string_view short_ = "";
        std::string_view env    = "";
        std::string_view help   = "";
        std::string_view oneof  = "";

        int field_number = 0;

        bool skipmissing = false;
        bool omitempty   = false;
        bool noserde     = false;
        bool positional  = false;

        // for proto
        bool fixed  = false;
        bool zigzag = false;
        bool packed = true;

        constexpr TagInfo() = default;

        template <size_t N>
        constexpr TagInfo(const char (&str)[N])
            : TagInfo(std::string_view(str, N - 1)) {}

        constexpr TagInfo(const char *str)
            : TagInfo(std::string_view(str)) {}

        constexpr TagInfo(std::string_view sv) {
            bool first = true;

            while (!sv.empty()) {
                size_t           next = sv.find(',');
                std::string_view part = sv.substr(0, next);
                constexpr char   ws[] = " \t\n\r\f\v";

                size_t start = part.find_first_not_of(ws);
                size_t end   = part.find_last_not_of(ws);

                part = start == std::string_view::npos ? std::string_view{} : part.substr(start, end - start + 1);

                if (first) {
                    first = false;
                    key   = part;
                    for (size_t i = 0; i < part.length(); ++i)
                        if (part[i] >= '0' && part[i] <= '9')
                            field_number = field_number * 10 + (part[i] - '0');
                        else
                            break;
                } else if (part == "fixed")
                    fixed = true;
                else if (part == "zigzag")
                    zigzag = true;
                else if (part == "packed=false")
                    packed = false;
                else if (std::string_view f = "field_number="; part.size() >= f.size() && part.compare(0, f.size(), f) == 0) {
                    std::string_view s = part.substr(f.size());
                    for (size_t i = 0; i < s.length(); ++i)
                        if (s[i] >= '0' && s[i] <= '9')
                            field_number = field_number * 10 + (s[i] - '0');
                        else
                            break;
                } else if (std::string_view e = "env="; part.size() >= e.size() && part.compare(0, e.size(), e) == 0)
                    env = part.substr(e.size());
                else if (part == "skipmissing")
                    skipmissing = true;
                else if (part == "omitempty")
                    omitempty = true;
                else if (part == "noserde")
                    noserde = true;
                else if (part == "positional")
                    positional = true;
                else if (std::string_view h = "short="; part.size() >= h.size() && part.compare(0, h.size(), h) == 0)
                    short_ = part.substr(h.size());
                else if (std::string_view h = "help="; part.size() >= h.size() && part.compare(0, h.size(), h) == 0)
                    help = part.substr(h.size());
                else if (std::string_view h = "oneof="; part.size() >= h.size() && part.compare(0, h.size(), h) == 0)
                    oneof = part.substr(h.size());

                if (next == std::string_view::npos)
                    break;
                sv.remove_prefix(next + 1);
            }
        }

        void operator=(const char *)     = delete;
        void operator=(std::string_view) = delete;
    };

    CPX_EXPORT template <typename T, typename TI>
    struct TagInfoFor {
        T  value;
        TI ti;

        constexpr TagInfoFor(T value, TI ti)
            : value(value)
            , ti(ti) {}
    };

    CPX_EXPORT template <typename T, typename TI>
    constexpr auto tag_tie(T &val, TI &ti) {
        return TagInfoFor<T &, TI &>(val, ti);
    }

    CPX_EXPORT class TagInfoBuilder {
        TagInfo t = {};

    public:
        constexpr TagInfoBuilder(std::string_view key) {
            t.key = key;
        }

        constexpr operator TagInfo() const {
            return t;
        }

        constexpr TagInfoBuilder &env(std::string_view env) {
            t.env = env;
            return *this;
        }

        constexpr TagInfoBuilder &help(std::string_view help) {
            t.help = help;
            return *this;
        }

        constexpr TagInfoBuilder &field_number(int field_number) {
            t.field_number = field_number;
            return *this;
        }

        constexpr TagInfoBuilder &skipmissing(bool skipmissing = true) {
            t.skipmissing = skipmissing;
            return *this;
        }

        constexpr TagInfoBuilder &omitempty(bool omitempty = true) {
            t.omitempty = omitempty;
            return *this;
        }

        constexpr TagInfoBuilder &noserde(bool noserde = true) {
            t.noserde = noserde;
            return *this;
        }

        constexpr TagInfoBuilder &positional(bool positional = true) {
            t.noserde = positional;
            return *this;
        }

        constexpr TagInfoBuilder &fixed(bool fixed = true) {
            t.fixed = fixed;
            return *this;
        }

        constexpr TagInfoBuilder &zigzag(bool zigzag = true) {
            t.zigzag = zigzag;
            return *this;
        }

        constexpr TagInfoBuilder &packed(bool packed = true) {
            t.packed = packed;
            return *this;
        }
    };

    CPX_EXPORT template <typename T>
    constexpr TagInfo get_tag_info(const T &field, std::string_view tag) {
        std::string_view sv;
        if constexpr (is_tagged_v<T>)
            sv = field.get_tag(tag);

        return {sv};
    }
} // namespace cpx

namespace cpx::detail {
    template <typename T>
    struct is_tag_info_for : std::false_type {};

    template <typename T, typename TI>
    struct is_tag_info_for<TagInfoFor<T, TI>> : std::true_type {};

    template <typename T>
    inline constexpr bool is_tag_info_for_v = is_tag_info_for<T>::value;


    template <typename T>
    struct tuple_has_any_tagged_type : std::false_type {};

    template <typename T, typename... Ts>
    struct tuple_has_any_tagged_type<std::tuple<T, Ts...>>
        : std::bool_constant<
              is_tag_info_for_v<std::decay_t<T>> || is_tagged_v<std::decay_t<T>> ||
              ((is_tag_info_for_v<std::decay_t<Ts>> || is_tagged_v<std::decay_t<Ts>>) || ...)> {};

    template <typename T, typename... Ts>
    struct tuple_has_any_tagged_type<const std::tuple<T, Ts...>>
        : std::bool_constant<
              is_tag_info_for_v<std::decay_t<T>> || is_tagged_v<std::decay_t<T>> ||
              ((is_tag_info_for_v<std::decay_t<Ts>> || is_tagged_v<std::decay_t<Ts>>) || ...)> {};

    template <typename T>
    inline constexpr bool tuple_has_any_tagged_type_v = tuple_has_any_tagged_type<T>::value;


    template <typename T>
    decltype(auto) get_underlying_value(T &value) {
        if constexpr (is_tagged_v<std::decay_t<decltype(value)>>)
            return value.get_value();
        else if constexpr (is_tag_info_for_v<std::decay_t<decltype(value)>>)
            return value.value;
        else
            return value;
    }

    template <typename, typename = void>
    struct has_empty : std::false_type {};

    template <typename T>
    struct has_empty<T, std::void_t<decltype(std::declval<const T &>().empty())>> : std::true_type {};

    template <typename T>
    std::enable_if_t<!has_empty<T>::value, bool> is_empty_value(const T &value) {
        if constexpr (is_optional<T>::value)
            return !value.has_value();
        else if constexpr (std::is_arithmetic_v<T>)
            return !bool(value);
        else
            return false;
    }

    template <typename T>
    std::enable_if_t<has_empty<T>::value, bool> is_empty_value(const T &value) {
        return value.empty();
    }
} // namespace cpx::detail

#endif
