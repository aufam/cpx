#ifndef CPX_TAG_H
#define CPX_TAG_H

#include <cpx/optional.h>
#include <cpx/tuple.h>
#include <string_view>
#include <variant>

namespace cpx {
    CPX_EXPORT struct Tag {
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

        constexpr Tag() = default;

        template <size_t N>
        constexpr Tag(const char (&str)[N])
            : Tag(std::string_view(str, N - 1)) {}

        constexpr Tag(const char *str)
            : Tag(std::string_view(str)) {}

        constexpr Tag(std::string_view sv) {
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
    struct TagFor {
        T  value;
        TI tag;

        constexpr TagFor(T value, TI tag)
            : value(value)
            , tag(tag) {}
    };

    CPX_EXPORT class TagBuilder {
        Tag t = {};

    public:
        constexpr TagBuilder(std::string_view key) {
            t.key = key;
        }

        constexpr operator const Tag &() const {
            return t;
        }

        constexpr TagBuilder &env(std::string_view env) {
            t.env = env;
            return *this;
        }

        constexpr TagBuilder &help(std::string_view help) {
            t.help = help;
            return *this;
        }

        constexpr TagBuilder &field_number(int field_number) {
            t.field_number = field_number;
            return *this;
        }

        constexpr TagBuilder &skipmissing(bool skipmissing = true) {
            t.skipmissing = skipmissing;
            return *this;
        }

        constexpr TagBuilder &omitempty(bool omitempty = true) {
            t.omitempty = omitempty;
            return *this;
        }

        constexpr TagBuilder &noserde(bool noserde = true) {
            t.noserde = noserde;
            return *this;
        }

        constexpr TagBuilder &positional(bool positional = true) {
            t.noserde = positional;
            return *this;
        }

        constexpr TagBuilder &fixed(bool fixed = true) {
            t.fixed = fixed;
            return *this;
        }

        constexpr TagBuilder &zigzag(bool zigzag = true) {
            t.zigzag = zigzag;
            return *this;
        }

        constexpr TagBuilder &packed(bool packed = true) {
            t.packed = packed;
            return *this;
        }
    };

    CPX_EXPORT template <auto Member>
    struct FieldTag {
        Tag tag;
    };

    CPX_EXPORT template <auto Member>
    struct _Field {
        template <size_t N>
        constexpr FieldTag<Member> operator=(const char (&str)[N]) const {
            return {Tag(str)};
        }

        constexpr FieldTag<Member> operator=(const TagBuilder t) const {
            return {t};
        }

        constexpr FieldTag<Member> operator=(const Tag &t) const {
            return {t};
        }
    };

    CPX_EXPORT template <auto Member>
    inline constexpr _Field<Member> field{};

    CPX_EXPORT template <typename T>
    struct _FieldFor {
        T value;

        template <size_t N>
        constexpr TagFor<T, Tag> operator=(const char (&str)[N]) const {
            return TagFor<T, Tag>(value, Tag(str));
        }

        constexpr TagFor<T, Tag> operator=(const TagBuilder &t) const {
            return {value, t};
        }

        constexpr TagFor<T, const Tag &> operator=(const Tag &t) const {
            return {value, t};
        }

        constexpr TagFor<T, Tag> operator=(Tag &&t) const {
            return {value, t};
        }
    };

    CPX_EXPORT template <typename T>
    constexpr auto field_ref(T &value) {
        return _FieldFor<T &>{value};
    }

    CPX_EXPORT template <typename... T, typename... TI>
    constexpr auto field_ref(std::tuple<TagFor<T &, TI>...> &&value) {
        return field_ref(value);
    }

    template <typename T, auto... Member, std::size_t... i>
    constexpr auto apply_field_tags(const T &v, const std::tuple<FieldTag<Member>...> &fields, std::index_sequence<i...>) {
        return std::make_tuple((field_ref(v.*Member) = std::get<i>(fields).tag)...);
    }

    template <typename T, auto... Member, std::size_t... i>
    constexpr auto apply_field_tags(T &v, const std::tuple<FieldTag<Member>...> &fields, std::index_sequence<i...>) {
        return std::make_tuple((field_ref(v.*Member) = std::get<i>(fields).tag)...);
    }

    CPX_EXPORT template <typename T, auto... Member>
    constexpr auto apply_field_tags(const T &v, const std::tuple<FieldTag<Member>...> &fields) {
        return apply_field_tags(v, fields, std::index_sequence_for<FieldTag<Member>...>{});
    }

    CPX_EXPORT template <typename T, auto... Member>
    constexpr auto apply_field_tags(T &v, const std::tuple<FieldTag<Member>...> &fields) {
        return apply_field_tags(v, fields, std::index_sequence_for<FieldTag<Member>...>{});
    }
} // namespace cpx

namespace cpx::detail {
    template <typename T>
    struct is_tag_for : std::false_type {};

    template <typename T, typename TI>
    struct is_tag_for<TagFor<T, TI>> : std::true_type {};

    template <typename T>
    inline constexpr bool is_tag_for_v = is_tag_for<T>::value;

    template <typename T>
    constexpr decltype(auto) get_tag(const T &field) {
        if constexpr (cpx::detail::is_tag_for_v<T>)
            return field.tag;
        else
            return cpx::Tag{};
    }

    template <typename T>
    struct tuple_has_any_tagged_type : std::false_type {};

    template <typename T, typename... Ts>
    struct tuple_has_any_tagged_type<std::tuple<T, Ts...>>
        : std::bool_constant<is_tag_for_v<std::decay_t<T>> || ((is_tag_for_v<std::decay_t<Ts>>) || ...)> {};

    template <typename T, typename... Ts>
    struct tuple_has_any_tagged_type<const std::tuple<T, Ts...>>
        : std::bool_constant<is_tag_for_v<std::decay_t<T>> || ((is_tag_for_v<std::decay_t<Ts>>) || ...)> {};

    template <typename T>
    inline constexpr bool tuple_has_any_tagged_type_v = tuple_has_any_tagged_type<T>::value;

    template <typename T>
    decltype(auto) get_underlying_value(T &value) {
        if constexpr (is_tag_for_v<std::decay_t<decltype(value)>>)
            return value.value;
        else
            return value;
    }

    template <typename T, typename = void>
    struct empty_trait {
        static constexpr bool is_empty(const T &) {
            return false;
        }
    };

    template <typename T>
    struct empty_trait<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
        static constexpr bool is_empty(const T &v) {
            return v == T(0);
        }
    };

    template <typename T>
    struct empty_trait<T, std::void_t<decltype(std::declval<const T &>().empty())>> {
        static constexpr bool is_empty(const T &v) {
            return v.empty();
        }
    };

    template <typename T>
    struct empty_trait<std::optional<T>> {
        static constexpr bool is_empty(const std::optional<T> &v) {
            return !v.has_value();
        }
    };

    template <typename... T>
    struct empty_trait<std::variant<T...>> {
        static constexpr bool is_empty(const std::variant<T...> &v) {
            return std::visit([](const auto &v) { return empty_trait<std::decay_t<decltype(v)>>::is_empty(v); }, v);
        }
    };

    template <typename T>
    constexpr bool is_empty_value(const T &value) {
        return empty_trait<T>::is_empty(value);
    }

    namespace test::test_is_empty_value {
        inline constexpr std::variant<int, std::string_view> v = "";

        struct foo {
            constexpr bool empty() const {
                return true;
            }
        };

        static_assert(is_empty_value(v));
        static_assert(is_empty_value(foo{}));
    } // namespace test::test_is_empty_value
} // namespace cpx::detail

#endif
