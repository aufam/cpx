#ifndef CPX_MSGPACK_H
#define CPX_MSGPACK_H

#include <cpx/reflect.h>
#include <cpx/extend.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <optional>
#include <variant>

#include <msgpack.hpp>

namespace cpx::msgpack {
    template <typename T>
    constexpr decltype(auto) get_tag_info(const T &field) {
        if constexpr (cpx::detail::is_tag_info_for_v<T>)
            return field.ti;
        else
            return cpx::get_tag_info(field, "msgpack");
    }

    template <typename T, typename Enable = void>
    struct Reflect : std::false_type {
        using const_type = type;
    };

    template <typename T>
    struct has_reflect : std::bool_constant<(Reflect<T>::value || cpx::has_reflect_v<T>) && !cpx::is_time_v<T>> {};

    template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    template <typename T>
    using reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::type, cpx::reflect_t<T>>;

    template <typename T>
    using const_reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::const_type, cpx::const_reflect_t<T>>;

    template <typename T>
    constexpr decltype(auto) reflect_of(T &v) {
        if constexpr (Reflect<std::remove_const_t<T>>::value)
            return Reflect<std::remove_const_t<T>>::of(v);
        else
            return cpx::reflect_of(v);
    }

    template <typename OS, typename From>
    using Serialize = cpx::serde::Serialize<::msgpack::packer<OS>, From>;

    template <typename To>
    using Dump = cpx::serde::Dump<::msgpack::packer<To>, To>;

    template <typename T>
    [[nodiscard]]
    std::string dump(const T &val);

    template <typename OS, typename T>
    void dump(OS &os, const T &val);
} // namespace cpx::msgpack

template <typename OS, typename T>
struct cpx::serde::Serialize<::msgpack::packer<OS>, T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>> {
    ::msgpack::packer<OS> &doc;

    void from(T v) const {
        if constexpr (std::is_same_v<T, bool>)
            v ? doc.pack_true() : doc.pack_false();
        else if constexpr (std::is_same_v<T, double>) {
            doc.pack_double(v);
        } else if constexpr (std::is_same_v<T, float>) {
            doc.pack_float(v);
        } else {
            doc.pack(v);
        }
    }
};

template <typename OS, typename CT>
struct cpx::serde::Serialize<::msgpack::packer<OS>, std::basic_string_view<char, CT>> {
    ::msgpack::packer<OS> &doc;

    void from(const std::basic_string_view<char, CT> &v) const {
        doc.pack_str(v.size());
        doc.pack_str_body(v.data(), v.size());
    }

    void from_bin(const std::basic_string_view<char, CT> &v) const {
        doc.pack_bin(v.size());
        doc.pack_bin_body(v.data(), v.size());
    }
};

template <typename OS, typename CT, typename A>
struct cpx::serde::Serialize<::msgpack::packer<OS>, std::basic_string<char, CT, A>> {
    ::msgpack::packer<OS> &doc;

    void from(const std::basic_string<char, CT, A> &v) const {
        Serialize<::msgpack::packer<OS>, std::basic_string_view<char, CT>>{doc}.from(v);
    }

    void from_bin(const std::basic_string<char, CT, A> &v) const {
        Serialize<::msgpack::packer<OS>, std::basic_string_view<char, CT>>{doc}.from_bin(v);
    }
};

template <typename OS, typename A>
struct cpx::serde::Serialize<::msgpack::packer<OS>, std::vector<uint8_t, A>> {
    ::msgpack::packer<OS> &doc;

    void from(const std::vector<uint8_t, A> &v) const {
        doc.pack_bin(v.size());
        doc.pack_bin_body(static_cast<const char *>(v.data()), v.size());
    }
};

template <typename OS, size_t N>
struct cpx::serde::Serialize<::msgpack::packer<OS>, std::array<uint8_t, N>> {
    ::msgpack::packer<OS> &doc;

    void from(const std::array<uint8_t, N> &v) const {
        doc.pack_bin(v.size());
        doc.pack_bin_body(static_cast<const char *>(v.data()), v.size());
    }
};

template <typename OS, typename T>
struct cpx::serde::Serialize<
    ::msgpack::packer<OS>,
    std::optional<T>,
    std::enable_if_t<cpx::serde::is_serializable_v<::msgpack::packer<OS>, T>>> {
    ::msgpack::packer<OS> &doc;

    void from(const std::optional<T> &v) const {
        if (v.has_value())
            Serialize<::msgpack::packer<OS>, T>{doc}.from(*v);
        else
            doc.pack_nil();
    }
};

template <typename OS, typename... T>
struct cpx::serde::Serialize<
    ::msgpack::packer<OS>,
    std::variant<T...>,
    std::enable_if_t<(cpx::serde::is_serializable_v<::msgpack::packer<OS>, T> && ...)>> {
    ::msgpack::packer<OS> &doc;

    void from(const std::variant<T...> &v) const {
        return std::visit(
            [](const auto &var) { return Serialize<::msgpack::packer<OS>, std::decay_t<decltype(var)>>{}.from(var); }, v
        );
    }
};

template <typename OS, typename T, size_t N>
struct cpx::serde::Serialize<::msgpack::packer<OS>, std::array<T, N>, std::enable_if_t<!std::is_same_v<T, uint8_t>>> {
    ::msgpack::packer<OS> &doc;

    void from(const std::array<T, N> &v) const {
        doc.pack_array(v.size());
        for (auto &item : v)
            Serialize<::msgpack::packer<OS>, T>{doc}.from(item);
    }
};

template <typename OS, typename T, typename A>
struct cpx::serde::Serialize<::msgpack::packer<OS>, std::vector<T, A>, std::enable_if_t<!std::is_same_v<T, uint8_t>>> {
    ::msgpack::packer<OS> &doc;

    void from(const std::vector<T, A> &v) const {
        doc.pack_array(v.size());
        for (auto &item : v)
            Serialize<::msgpack::packer<OS>, T>{doc}.from(item);
    }
};

template <typename OS, typename K, typename T, typename H, typename P, typename A>
struct cpx::serde::Serialize<
    ::msgpack::packer<OS>,
    std::unordered_map<K, T, H, P, A>,
    std::enable_if_t<
        cpx::serde::is_serializable_v<::msgpack::packer<OS>, K> || cpx::serde::is_serializable_v<::msgpack::packer<OS>, T>>> {
    ::msgpack::packer<OS> &doc;

    void from(const std::unordered_map<K, T, H, P, A> &v) const {
        doc.pack_map(v.size());
        for (auto &[k, v] : v) {
            Serialize<::msgpack::packer<OS>, K>{doc}.from(k);
            Serialize<::msgpack::packer<OS>, T>{doc}.from(v);
        }
    }
};

template <typename OS, typename... Ts>
struct cpx::serde::Serialize<::msgpack::packer<OS>, std::tuple<Ts...>> {
    ::msgpack::packer<OS> &doc;

    void from(const std::tuple<Ts...> &tpl) const {
        auto flattened        = cpx::flatten(tpl);
        using Tpl             = decltype(flattened);
        constexpr bool is_map = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;

        std::array<std::variant<std::monostate, int, std::string_view>, std::tuple_size_v<Tpl>> keys = {};

        size_t total = 0;
        tuple_for_each(flattened, [&](auto &item, const size_t i) {
            const cpx::TagInfo &t       = cpx::msgpack::get_tag_info(item);
            auto               &v       = cpx::detail::get_underlying_value(item);
            using T                     = std::decay_t<decltype(v)>;
            constexpr bool serializable = cpx::serde::is_serializable_v<::msgpack::packer<OS>, T>;

            if constexpr (is_map) {
                if constexpr (serializable) {
                    if (t.field_number > 0) {
                        total++;
                        keys[i] = t.field_number;
                    } else if (!t.key.empty()) {
                        total++;
                        keys[i] = t.key;
                    }
                }
            } else
                total += serializable;
        });

        if constexpr (is_map) {
            doc.pack_map(total);
        } else {
            doc.pack_array(total);
        }

        tuple_for_each(flattened, [&](auto &item, const size_t i) {
            auto &v = cpx::detail::get_underlying_value(item);
            using T = std::decay_t<decltype(v)>;

            constexpr bool serializable = cpx::serde::is_serializable_v<::msgpack::packer<OS>, T>;
            const std::variant<std::monostate, int, std::string_view> &t = keys[i];

            const auto idx = t.index();
            if constexpr (serializable) {
                if (idx == 1) {
                    Serialize<::msgpack::packer<OS>, int>{doc}.from(std::get<int>(t));
                } else if (idx == 2) {
                    Serialize<::msgpack::packer<OS>, std::string_view>{doc}.from(std::get<std::string_view>(t));
                }
                Serialize<::msgpack::packer<OS>, T>{doc}.from(v);
            }
        });
    }
};

template <typename OS, typename T>
struct cpx::serde::Serialize<::msgpack::packer<OS>, T, std::enable_if_t<cpx::msgpack::has_reflect_v<T>>> {
    ::msgpack::packer<OS> &doc;

    void from(const T &v) const {
        Serialize<::msgpack::packer<OS>, cpx::msgpack::const_reflect_t<T>>{}.from(cpx::msgpack::reflect_of(v));
    }
};

template <typename OS>
struct cpx::serde::Dump<::msgpack::packer<OS>, OS> {
    OS &os;

    template <typename T>
    OS &from(const T &v) const {
        ::msgpack::packer<OS> doc(os);
        Serialize<::msgpack::packer<OS>, T>{doc}.from(v);
        return os;
    }

    template <typename T>
    OS &operator<<(const T &v) const {
        return from(v);
    }
};

template <typename T>
[[nodiscard]]
std::string cpx::msgpack::dump(const T &val) {
    ::msgpack::sbuffer buf;
    cpx::msgpack::Dump<::msgpack::sbuffer>{buf}.from(val);
    return {buf.data(), buf.size()};
}

template <typename OS, typename T>
void cpx::msgpack::dump(OS &os, const T &val) {
    cpx::msgpack::Dump<OS>{os}.from(val);
}

namespace cpx::msgpack {
    constexpr struct IO {
        template <typename OS>
        friend cpx::msgpack::Dump<OS> operator<<(OS &os, const IO &) {
            return {os};
        }
    } io{};
} // namespace cpx::msgpack
#endif
