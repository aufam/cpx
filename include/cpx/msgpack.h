#ifndef CPX_MSGPACK_H
#define CPX_MSGPACK_H

#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <optional>
#include <variant>

#ifndef CPX_MODULE
#    include <msgpack.hpp>
#endif

namespace cpx::msgpack {
    CPX_EXPORT template <typename T>
    constexpr decltype(auto) get_tag_info(const T &field) {
        if constexpr (cpx::detail::is_tag_info_for_v<T>)
            return field.ti;
        else
            return cpx::get_tag_info(field, "msgpack");
    }

    CPX_EXPORT template <typename T, typename Enable = void>
    struct Reflect : std::false_type {
        using const_type = type;
    };

    CPX_EXPORT template <typename T>
    struct has_reflect
        : std::bool_constant<(Reflect<T>::value || cpx::has_reflect_v<T>) && !cpx::is_time_v<T> && !std::is_enum_v<T>> {};

    CPX_EXPORT template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    CPX_EXPORT template <typename T>
    using reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::type, cpx::reflect_t<T>>;

    CPX_EXPORT template <typename T>
    using const_reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::const_type, cpx::const_reflect_t<T>>;

    CPX_EXPORT template <typename T>
    constexpr decltype(auto) reflect_of(T &v) {
        if constexpr (Reflect<std::remove_const_t<T>>::value)
            return Reflect<std::remove_const_t<T>>::of(v);
        else
            return cpx::reflect_of(v);
    }

    CPX_EXPORT template <typename OS, typename From>
    using Serialize = cpx::serde::Serialize<::msgpack::packer<OS>, From>;

    CPX_EXPORT template <typename OS, typename To>
    using Deserialize = cpx::serde::Serialize<::msgpack::object, To>;

    CPX_EXPORT template <typename To>
    using Dump = cpx::serde::Dump<::msgpack::packer<To>, To>;

    CPX_EXPORT template <typename From>
    using Parse = cpx::serde::Parse<::msgpack::unpacker, From>;

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::string dump(const T &val);

    CPX_EXPORT template <typename OS, typename T>
    void dump(OS &os, const T &val);

    CPX_EXPORT template <typename T>
    void parse(std::string_view buffer, T &val);

    CPX_EXPORT template <typename T>
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(std::string_view buffer);

    CPX_EXPORT template <typename T>
    void parse(std::istream &, T &val);

    CPX_EXPORT template <typename T>
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(std::istream &);
} // namespace cpx::msgpack

// primitive types
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

template <typename T>
struct cpx::serde::Deserialize<::msgpack::object, T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>> {
    const ::msgpack::object &obj;

    void into(T &v) const {
        try {
            v = obj.as<T>();
        } catch (const ::msgpack::type_error &e) {
            throw type_mismatch_error("primitive", "unknown", e.what());
        }
    }
};

// enums
template <typename OS, typename T>
struct cpx::serde::Serialize<::msgpack::packer<OS>, T, std::enable_if_t<std::is_enum_v<T> && !cpx::msgpack::has_reflect_v<T>>> {
    ::msgpack::packer<OS> &doc;

    void from(T v) const {
        doc.pack(int(v));
    }
};

template <typename T>
struct cpx::serde::Deserialize<::msgpack::object, T, std::enable_if_t<std::is_enum_v<T> && !cpx::msgpack::has_reflect_v<T>>> {
    const ::msgpack::object &obj;

    void into(T &v) const {
        try {
            v = (T)obj.as<int>();
        } catch (const ::msgpack::type_error &e) {
            throw type_mismatch_error("primitive", "unknown", e.what());
        }
    }
};

// strings
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

template <typename CT>
struct cpx::serde::Deserialize<::msgpack::object, std::basic_string_view<char, CT>> {
    const ::msgpack::object &obj;

    void into(std::basic_string_view<char, CT> &v) const {
        if (obj.type != ::msgpack::type::STR)
            throw type_mismatch_error("str", "unknown");
        v = std::basic_string_view<char, CT>{obj.via.str.ptr, obj.via.str.size};
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

template <typename CT, typename A>
struct cpx::serde::Deserialize<::msgpack::object, std::basic_string<char, CT, A>> {
    const ::msgpack::object &obj;

    void into(std::basic_string<char, CT, A> &v) const {
        if (obj.type != ::msgpack::type::STR)
            throw type_mismatch_error("str", "unknown");
        v = std::basic_string<char, CT, A>{obj.via.str.ptr, obj.via.str.size};
    }
};

// bins
template <typename OS, typename A>
struct cpx::serde::Serialize<::msgpack::packer<OS>, std::vector<uint8_t, A>> {
    ::msgpack::packer<OS> &doc;

    void from(const std::vector<uint8_t, A> &v) const {
        doc.pack_bin(v.size());
        doc.pack_bin_body(static_cast<const char *>(v.data()), v.size());
    }
};

template <typename A>
struct cpx::serde::Deserialize<::msgpack::object, std::vector<uint8_t, A>> {
    const ::msgpack::object &obj;

    void into(std::vector<uint8_t, A> &v) const {
        if (obj.type != ::msgpack::type::BIN)
            throw type_mismatch_error("bin", "unknown");
        auto begin = reinterpret_cast<const uint8_t *>(obj.via.bin.ptr);
        v          = std::vector<uint8_t, A>{begin, begin + obj.via.bin.size};
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

template <size_t N>
struct cpx::serde::Deserialize<::msgpack::object, std::array<uint8_t, N>> {
    const ::msgpack::object &obj;

    void into(std::array<uint8_t, N> &v) const {
        if (obj.type != ::msgpack::type::BIN)
            throw type_mismatch_error("bin", "unknown");
        if (N != obj.via.bin.size)
            throw size_mismatch_error(N, obj.via.bin.size);
        ::memcpy(v.data(), obj.via.bin.ptr, N);
    }
};

// optional
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

template <typename T>
struct cpx::serde::Deserialize<
    ::msgpack::object,
    std::optional<T>,
    std::enable_if_t<cpx::serde::is_deserializable_v<::msgpack::object, T> && std::is_default_constructible_v<T>>> {
    const ::msgpack::object &obj;

    void into(std::optional<T> &v) const {
        if (obj.is_nil())
            v = std::nullopt;
        else {
            if (!v.has_value())
                v.emplace();
            Deserialize<::msgpack::object, T>{obj}.into(*v);
        }
    }
};

// variant
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

template <typename... T>
struct cpx::serde::Deserialize<
    ::msgpack::object,
    std::variant<T...>,
    std::enable_if_t<((cpx::serde::is_deserializable_v<::msgpack::object, T> && std::is_default_constructible_v<T>) && ...)>> {
    const ::msgpack::object &obj;

    void into(std::variant<T...> &v) const {
        bool        done = false;
        std::string type_names;
        (
            [&]() {
                try {
                    if (!done) {
                        auto element = T{};
                        Deserialize<::msgpack::object, T>{obj}.into(element);
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
            throw type_mismatch_error(type_names, "unknown");
        }
    }
};

// array
template <typename OS, typename T, size_t N>
struct cpx::serde::Serialize<
    ::msgpack::packer<OS>,
    std::array<T, N>,
    std::enable_if_t<!std::is_same_v<T, uint8_t> && cpx::serde::is_serializable_v<::msgpack::packer<OS>, T>>> {
    ::msgpack::packer<OS> &doc;

    void from(const std::array<T, N> &v) const {
        doc.pack_array(v.size());
        for (auto &item : v)
            Serialize<::msgpack::packer<OS>, T>{doc}.from(item);
    }
};

template <typename T, size_t N>
struct cpx::serde::Deserialize<
    ::msgpack::object,
    std::array<T, N>,
    std::enable_if_t<!std::is_same_v<T, uint8_t> && cpx::serde::is_deserializable_v<::msgpack::object, T>>> {
    const ::msgpack::object &obj;

    void into(std::array<T, N> &v) const {
        if (obj.type != ::msgpack::type::ARRAY)
            throw type_mismatch_error("array", "unknown");
        if (obj.via.array.size != N)
            throw size_mismatch_error(N, obj.via.array.size);

        for (size_t i = 0; i < obj.via.array.size; ++i) {
            try {
                Deserialize<::msgpack::object, T>{obj.via.array.ptr[i]}.into(v[i]);
            } catch (error &e) {
                e.add_context(i);
                throw;
            }
        }
    }
};

template <typename OS, typename T, typename A>
struct cpx::serde::Serialize<
    ::msgpack::packer<OS>,
    std::vector<T, A>,
    std::enable_if_t<!std::is_same_v<T, uint8_t> && cpx::serde::is_serializable_v<::msgpack::packer<OS>, T>>> {
    ::msgpack::packer<OS> &doc;

    void from(const std::vector<T, A> &v) const {
        doc.pack_array(v.size());
        for (auto &item : v)
            Serialize<::msgpack::packer<OS>, T>{doc}.from(item);
    }
};

template <typename T, typename A>
struct cpx::serde::Deserialize<
    ::msgpack::object,
    std::vector<T, A>,
    std::enable_if_t<!std::is_same_v<T, uint8_t> && cpx::serde::is_deserializable_v<::msgpack::object, T>>> {
    const ::msgpack::object &obj;

    void into(std::vector<T, A> &v) const {
        if (obj.type != ::msgpack::type::ARRAY)
            throw type_mismatch_error("array", "unknown");

        v.resize(obj.via.array.size);
        for (size_t i = 0; i < obj.via.array.size; ++i) {
            try {
                Deserialize<::msgpack::object, T>{obj.via.array.ptr[i]}.into(v[i]);
            } catch (error &e) {
                e.add_context(i);
                throw;
            }
        }
    }
};

template <typename OS, typename K, typename T, typename H, typename P, typename A>
struct cpx::serde::Serialize<
    ::msgpack::packer<OS>,
    std::unordered_map<K, T, H, P, A>,
    std::enable_if_t<
        cpx::serde::is_serializable_v<::msgpack::packer<OS>, K> && cpx::serde::is_serializable_v<::msgpack::packer<OS>, T>>> {
    ::msgpack::packer<OS> &doc;

    void from(const std::unordered_map<K, T, H, P, A> &v) const {
        doc.pack_map(v.size());
        for (auto &[k, v] : v) {
            Serialize<::msgpack::packer<OS>, K>{doc}.from(k);
            Serialize<::msgpack::packer<OS>, T>{doc}.from(v);
        }
    }
};

template <typename K, typename T, typename H, typename P, typename A>
struct cpx::serde::Deserialize<
    ::msgpack::object,
    std::unordered_map<K, T, H, P, A>,
    std::enable_if_t<
        cpx::serde::is_deserializable_v<::msgpack::object, K> && cpx::serde::is_deserializable_v<::msgpack::object, T> &&
        std::is_default_constructible_v<K> && std::is_default_constructible_v<T>>> {
    const ::msgpack::object &obj;

    void into(std::unordered_map<K, T, H, P, A> &v) const {
        if (obj.type != ::msgpack::type::MAP)
            throw type_mismatch_error("map", "unknown");

        for (size_t i = 0; i < obj.via.map.size; ++i) {
            K key = {};
            T val = {};
            try {
                Deserialize<::msgpack::object, K>{obj.via.map.ptr[i].key}.into(key);
                Deserialize<::msgpack::object, T>{obj.via.map.ptr[i].val}.into(val);
            } catch (error &e) {
                e.add_context(i);
                throw;
            }
            v.emplace(std::move(key), std::move(val));
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

template <typename... Ts>
struct cpx::serde::Deserialize<::msgpack::object, std::tuple<Ts...>> {
    const ::msgpack::object &obj;

    void into(std::tuple<Ts...> &tpl) const {
        auto flattened        = cpx::flatten(tpl);
        using Tpl             = decltype(flattened);
        constexpr bool is_map = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;

        if (is_map && obj.type != ::msgpack::type::MAP)
            throw type_mismatch_error("map", "unknown");
        if (!is_map && obj.type != ::msgpack::type::ARRAY)
            throw type_mismatch_error("array", "unknown");

        size_t idx = 0;
        tuple_for_each(flattened, [&](auto &item, const size_t) {
            const cpx::TagInfo &t         = cpx::msgpack::get_tag_info(item);
            auto               &v         = cpx::detail::get_underlying_value(item);
            using T                       = std::decay_t<decltype(v)>;
            constexpr bool deserializable = cpx::serde::is_deserializable_v<::msgpack::object, T>;

            if constexpr (!deserializable)
                return;

            const size_t i = idx++;
            try {
                if constexpr (is_map) {
                    if (obj.via.map.ptr[i].key.type == ::msgpack::type::POSITIVE_INTEGER) {
                        auto got = obj.via.map.ptr[i].key.as<int>();
                        if (t.field_number != got) {
                            throw type_mismatch_error(std::to_string(t.field_number), std::to_string(got), "on map key");
                        }
                    } else if (obj.via.map.ptr[i].key.type == ::msgpack::type::STR) {
                        auto &key = obj.via.map.ptr[i].key.via.str;
                        auto  got = std::string_view(key.ptr, key.size);
                        if (t.key != got) {
                            throw type_mismatch_error(std::string(t.key), std::string(got), "on map key");
                        }
                    } else {
                        throw type_mismatch_error("int|str", "unknown");
                    }
                    Deserialize<::msgpack::object, T>{obj.via.map.ptr[i].val}.into(v);
                } else {
                    Deserialize<::msgpack::object, T>{obj.via.array.ptr[i]}.into(v);
                }
            } catch (error &e) {
                e.add_context(i);
                throw;
            }
        });
    }
};

template <typename OS, typename T>
struct cpx::serde::Serialize<::msgpack::packer<OS>, T, std::enable_if_t<cpx::msgpack::has_reflect_v<T>>> {
    ::msgpack::packer<OS> &doc;

    void from(const T &v) const {
        Serialize<::msgpack::packer<OS>, cpx::msgpack::const_reflect_t<T>>{doc}.from(cpx::msgpack::reflect_of(v));
    }
};

template <typename T>
struct cpx::serde::Deserialize<::msgpack::object, T, std::enable_if_t<cpx::msgpack::has_reflect_v<T>>> {
    const ::msgpack::object &obj;

    void into(T &v) const {
        decltype(auto) r = cpx::msgpack::reflect_of(v);
        Deserialize<::msgpack::object, cpx::msgpack::reflect_t<T>>{obj}.into(r);
    }
};

template <typename OS>
struct cpx::serde::Serialize<::msgpack::packer<OS>, std::tm> {
    ::msgpack::packer<OS> &doc;

    void from(const std::tm &v) const {
        std::tm tm  = v;
        time_t  sec = timegm(&tm);
        Serialize<::msgpack::packer<OS>, time_t>{doc}.from(sec);
    }
};

template <>
struct cpx::serde::Deserialize<::msgpack::object, std::tm> {
    const ::msgpack::object &obj;

    void into(std::tm &v) const {
        time_t sec = {};
        Deserialize<::msgpack::object, time_t>{obj}.into(sec);
        v = *std::gmtime(&sec);
    }
};

template <typename OS>
struct cpx::serde::Serialize<::msgpack::packer<OS>, std::timespec> {
    ::msgpack::packer<OS> &doc;

    void from(const std::timespec &v) const {
        auto r = std::tie(v.tv_sec, v.tv_nsec);
        Serialize<::msgpack::packer<OS>, decltype(r)>{doc}.from(r);
    }
};

template <>
struct cpx::serde::Deserialize<::msgpack::object, std::timespec> {
    const ::msgpack::object &obj;

    void into(std::timespec &v) const {
        auto r = std::tie(v.tv_sec, v.tv_nsec);
        Deserialize<::msgpack::object, decltype(r)>{obj}.into(r);
    }
};


template <>
struct cpx::serde::Parse<::msgpack::unpacker, std::string_view> {
    std::string_view buffer;

    template <typename T>
    void into(T &v) const {
        ::msgpack::object_handle oh  = ::msgpack::unpack(buffer.data(), buffer.size());
        ::msgpack::object        obj = oh.get();
        Deserialize<::msgpack::object, T>{obj}.into(v);
    }
};

template <>
struct cpx::serde::Parse<::msgpack::unpacker, std::istream> {
    std::istream &stream;

    template <typename T>
    std::istream &into(T &v) const {
        ::msgpack::unpacker      pac;
        ::msgpack::object_handle oh;

        while (!pac.next(oh)) {
            pac.reserve_buffer(4096);
            stream.read(pac.buffer(), 4096);
            auto n = stream.gcount();

            if (n == 0)
                throw std::runtime_error("EOF");

            pac.buffer_consumed(n);
        }

        ::msgpack::object obj = oh.get();
        Deserialize<::msgpack::object, T>{obj}.into(v);
        return stream;
    }

    template <typename T>
    std::istream &operator>>(T &v) const {
        return into(v);
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

template <typename T>
void cpx::msgpack::parse(std::string_view buffer, T &val) {
    Parse<std::string_view>{buffer}.into(val);
}

template <typename T>
std::enable_if_t<std::is_default_constructible_v<T>, T> cpx::msgpack::parse(std::string_view buffer) {
    T val = {};
    Parse<std::string_view>{buffer}.into(val);
    return val;
}

template <typename T>
void cpx::msgpack::parse(std::istream &stream, T &val) {
    Parse<std::istream>{stream}.into(val);
}

template <typename T>
std::enable_if_t<std::is_default_constructible_v<T>, T> cpx::msgpack::parse(std::istream &stream) {
    T val = {};
    Parse<std::istream>{stream}.into(val);
    return val;
}

namespace cpx::msgpack {
    CPX_EXPORT constexpr struct IO {
        template <typename OS>
        friend cpx::msgpack::Dump<OS> operator<<(OS &os, const IO &) {
            return {os};
        }
        friend cpx::msgpack::Parse<std::istream> operator>>(std::istream &is, const IO &) {
            return {is};
        }
    } io{};
} // namespace cpx::msgpack
#endif
