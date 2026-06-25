#ifndef CPX_JSON_YYJSON_H
#define CPX_JSON_YYJSON_H

#include <cpx/json/json.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <cpx/defer.h>
#include <array>
#include <variant>
#include <vector>
#include <tuple>
#include <unordered_map>

#ifndef YYJSON_H
#    include <yyjson.h>
#endif

#define SERIALIZE(...)      cpx::serde::Serialize<yyjson_mut_val, __VA_ARGS__>
#define DESERIALIZE(...)    cpx::serde::Deserialize<yyjson_val, __VA_ARGS__>
#define SERIALIZABLE(...)   cpx::serde::is_serializable_v<yyjson_mut_val, __VA_ARGS__>
#define DESERIALIZABLE(...) cpx::serde::is_deserializable_v<yyjson_val, __VA_ARGS__>
#define DUMP(...)           cpx::serde::Dump<yyjson_mut_doc, __VA_ARGS__>
#define PARSE(...)          cpx::serde::Parse<yyjson_doc, __VA_ARGS__>

namespace cpx::json::yy_json {
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
    void parse(const std::string &str, T &val, yyjson_read_flag = YYJSON_READ_NOFLAG);

    CPX_EXPORT template <typename T>
    void parse_from_file(const std::string &path, T &val, yyjson_read_flag = YYJSON_READ_NOFLAG);

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(const std::string &str, yyjson_read_flag = YYJSON_READ_NOFLAG);

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T>
    parse_from_file(const std::string &path, yyjson_read_flag = YYJSON_READ_NOFLAG);

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::string dump(const T &val, yyjson_write_flag = YYJSON_WRITE_NOFLAG, const yyjson_alc *alc = nullptr);

    CPX_EXPORT template <typename T>
    void dump(const T &&val, yyjson_write_flag = YYJSON_WRITE_NOFLAG, const yyjson_alc *alc = nullptr) = delete;
} // namespace cpx::json::yy_json

namespace cpx {
    CPX_EXPORT namespace yy_json = cpx::json::yy_json;
}

// bool
template <>
struct SERIALIZE(bool) {
    yyjson_mut_doc *doc;

    yyjson_mut_val *from(bool v) const {
        return yyjson_mut_bool(doc, v);
    }
};

template <>
struct DESERIALIZE(bool) {
    yyjson_val *val;

    void into(bool &v) const {
        if (!yyjson_is_bool(val))
            throw type_mismatch_error("bool", yyjson_get_type_desc(val));
        v = yyjson_get_bool(val);
    }
};

// sint
template <typename T>
struct SERIALIZE(T, std::enable_if_t<std::is_signed_v<T> && !std::is_same_v<T, bool> && !std::is_floating_point_v<T>>) {
    yyjson_mut_doc *doc;

    yyjson_mut_val *from(T v) const {
        return yyjson_mut_sint(doc, v);
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<std::is_signed_v<T> && !std::is_same_v<T, bool> && !std::is_floating_point_v<T>>) {
    yyjson_val *val;

    void into(T &v) const {
        if (!yyjson_is_sint(val) && !yyjson_is_uint(val))
            throw type_mismatch_error("sint", yyjson_get_type_desc(val));
        v = (T)yyjson_get_sint(val);
    }
};

// uint
template <typename T>
struct SERIALIZE(T, std::enable_if_t<std::is_unsigned_v<T> && !std::is_same_v<T, bool>>) {
    yyjson_mut_doc *doc;

    yyjson_mut_val *from(T v) const {
        return yyjson_mut_uint(doc, v);
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<std::is_unsigned_v<T> && !std::is_same_v<T, bool>>) {
    yyjson_val *val;

    void into(T &v) const {
        if (!yyjson_is_uint(val))
            throw type_mismatch_error("uint", yyjson_get_type_desc(val));
        v = (T)yyjson_get_uint(val);
    }
};

// float
template <typename T>
struct SERIALIZE(T, std::enable_if_t<std::is_floating_point_v<T>>) {
    yyjson_mut_doc *doc;

    yyjson_mut_val *from(T v) const {
        return yyjson_mut_real(doc, v);
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<std::is_floating_point_v<T>>) {
    yyjson_val *val;

    void into(T &v) const {
        if (!yyjson_is_real(val))
            throw type_mismatch_error("real", yyjson_get_type_desc(val));
        v = (T)yyjson_get_real(val);
    }
};

// string
template <typename CT>
struct SERIALIZE(std::basic_string_view<char, CT>) {
    yyjson_mut_doc *doc;

    yyjson_mut_val *from(std::basic_string_view<char, CT> v, bool owned = false) const {
        return owned ? yyjson_mut_strncpy(doc, v.data(), v.size()) : yyjson_mut_strn(doc, v.data(), v.size());
    }

    yyjson_mut_val *from_raw(std::basic_string_view<char, CT> v, yyjson_read_flag flag = 0, const yyjson_alc *alc = nullptr) {
        yyjson_read_err err;
        yyjson_doc     *doc = yyjson_read_opts(const_cast<char *>(v.data()), v.size(), flag, alc, &err);

        if (!doc)
            throw error(err.msg);
        auto _ = defer([&]() { yyjson_doc_free(doc); });

        yyjson_val *root = yyjson_doc_get_root(doc);
        return yyjson_val_mut_copy(this->doc, root);
    }
};

template <typename CT, typename A>
struct SERIALIZE(std::basic_string<char, CT, A>) {
    yyjson_mut_doc *doc;

    yyjson_mut_val *from(const std::basic_string<char, CT, A> &v) const {
        return SERIALIZE(std::basic_string_view<char, CT>){doc}.from(v, false);
    }
    yyjson_mut_val *from(std::basic_string<char, CT, A> &&v) const {
        return SERIALIZE(std::basic_string_view<char, CT>){doc}.from(v, true);
    }

    yyjson_mut_val *
    from_raw(const std::basic_string<char, CT, A> &v, yyjson_read_flag flag = 0, const yyjson_alc *alc = nullptr) const {
        return SERIALIZE(std::basic_string_view<char, CT>){doc}.from_raw(v, flag, alc);
    }
};

template <typename CT, typename A>
struct DESERIALIZE(std::basic_string<char, CT, A>) {
    yyjson_val *val;

    void into(std::basic_string<char, CT, A> &v) const {
        if (!yyjson_is_str(val))
            throw type_mismatch_error("string", yyjson_get_type_desc(val));
        v = yyjson_get_str(val);
    }

    void into_raw(std::basic_string<char, CT, A> &v, yyjson_read_flag flag = 0, const yyjson_alc *alc = nullptr) const {
        yyjson_mut_doc *doc    = yyjson_mut_doc_new(alc);
        yyjson_mut_val *copied = yyjson_val_mut_copy(doc, val);
        yyjson_mut_doc_set_root(doc, copied);

        size_t           len;
        yyjson_write_err err;
        const char      *str = yyjson_mut_write_opts(doc, flag, alc, &len, &err);
        if (!str)
            throw error(err.msg);

        auto _ = defer([&]() { ::free((void *)str); });
        v      = std::basic_string<char, CT, A>(str, len);
    }
};

// optional
template <typename T>
struct SERIALIZE(std::optional<T>, std::enable_if_t<SERIALIZABLE(T)>) {
    yyjson_mut_doc *doc;

    yyjson_mut_val *from(const std::optional<T> &v) const {
        if (!v.has_value())
            return yyjson_mut_null(doc);
        return SERIALIZE(T){doc}.from(*v);
    }
};

template <typename T>
struct DESERIALIZE(std::optional<T>, std::enable_if_t<std::is_default_constructible_v<T> && DESERIALIZABLE(T)>) {
    yyjson_val *val;

    void into(std::optional<T> &v) const {
        if (!val || yyjson_is_null(val))
            return void(v = std::nullopt);
        v = T{};
        DESERIALIZE(T){val}.into(*v);
    }
};

// array
template <typename T, size_t N>
struct SERIALIZE(std::array<T, N>, std::enable_if_t<SERIALIZABLE(T)>) {
    yyjson_mut_doc *doc;

    template <typename I>
    yyjson_mut_val *from(I begin, I end) const {
        yyjson_mut_val *arr = yyjson_mut_arr(doc);
        size_t          i   = 0;
        for (I ptr = begin; ptr != end; ++i, ++ptr)
            try {
                yyjson_mut_val *val = SERIALIZE(T){doc}.from(*ptr);
                yyjson_mut_arr_append(arr, val);
            } catch (error &e) {
                e.add_context(i);
                throw;
            }
        return arr;
    }

    yyjson_mut_val *from(const std::array<T, N> &v) const {
        return from(v.begin(), v.end());
    }
};

template <typename T, size_t N>
struct DESERIALIZE(std::array<T, N>, std::enable_if_t<DESERIALIZABLE(T)>) {
    yyjson_val *val;

    template <typename I>
    void into(I begin, I) const {
        auto        arr = this->val;
        size_t      idx, max;
        yyjson_val *val;
        I           ptr = begin;
        yyjson_arr_foreach(arr, idx, max, val) try {
            DESERIALIZE(T){val}.into(*ptr);
            ++ptr;
        } catch (error &e) {
            e.add_context(idx);
            throw;
        }
    }

    void into(std::array<T, N> &v) const {
        auto arr = this->val;
        if (!yyjson_is_arr(arr))
            throw type_mismatch_error("array", yyjson_get_type_desc(arr));
        if (auto n = yyjson_arr_size(arr); N != n)
            throw size_mismatch_error(N, n);

        into(v.begin(), v.end());
    }
};

// vector
template <typename T, typename A>
struct SERIALIZE(std::vector<T, A>, std::enable_if_t<SERIALIZABLE(T)>) {
    yyjson_mut_doc *doc;

    yyjson_mut_val *from(const std::vector<T, A> &v) const {
        return SERIALIZE(std::array<T, 1>){doc}.from(v.begin(), v.end());
    }
};

template <typename T, typename A>
struct DESERIALIZE(std::vector<T, A>, std::enable_if_t<std::is_default_constructible_v<T> && DESERIALIZABLE(T)>) {
    yyjson_val *val;

    void into(std::vector<T, A> &v) const {
        auto arr = this->val;
        if (!yyjson_is_arr(arr))
            throw type_mismatch_error("array", yyjson_get_type_desc(arr));

        v.resize(yyjson_arr_size(arr));
        DESERIALIZE(std::array<T, 1>){arr}.into(v.begin(), v.end());
    }
};

// tuple
template <typename... Ts>
struct SERIALIZE(std::tuple<Ts...>) {
    yyjson_mut_doc *doc;

    yyjson_mut_val *from(const std::tuple<Ts...> &tpl) const {
        auto flattened             = flatten(tpl);
        using Tpl                  = decltype(flattened);
        constexpr bool  is_obj     = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;
        yyjson_mut_val *obj_or_arr = is_obj ? yyjson_mut_obj(doc) : yyjson_mut_arr(doc);

        auto   oneofs      = std::array<std::string_view, std::tuple_size_v<Tpl>>{};
        size_t oneof_count = 0;
        size_t idx         = 0;
        tuple_for_each(flattened, [&](auto &item, const size_t) {
            const cpx::TagInfo &t = cpx::json::get_tag_info(item);
            auto               &v = cpx::detail::get_underlying_value(item);
            using T               = std::decay_t<decltype(v)>;

            if (!SERIALIZABLE(T) || (is_obj && t.key == ""))
                return;

            size_t i = idx++;
            if ((t.omitempty || !t.oneof.empty()) && cpx::detail::is_empty_value(v)) {
                if constexpr (!is_obj)
                    yyjson_mut_arr_append(obj_or_arr, yyjson_mut_null(doc));
                return;
            }

            if (!t.oneof.empty()) {
                for (size_t j = 0; j < oneof_count; ++j) {
                    if (oneofs[j] == t.oneof)
                        throw duplicate_oneof_error(t.oneof);
                }
                oneofs[oneof_count++] = t.oneof;
            }

            yyjson_mut_val *val = nullptr;
            try {
                if (t.noserde)
                    if constexpr (std::is_same_v<T, std::string>)
                        val = SERIALIZE(std::string){doc}.from_raw(v);
                    else
                        throw error("field with tag `noserde` can only be serialized from std::string");
                else {
                    if constexpr (SERIALIZABLE(T))
                        val = SERIALIZE(T){doc}.from(v);
                }
            } catch (error &e) {
                if constexpr (is_obj)
                    e.add_context(t.key);
                else
                    e.add_context(i);
                throw;
            }
            if constexpr (is_obj)
                yyjson_mut_obj_add(obj_or_arr, yyjson_mut_strn(doc, t.key.data(), t.key.size()), val);
            else
                yyjson_mut_arr_append(obj_or_arr, val);
        });

        return obj_or_arr;
    }
};

template <typename... Ts>
struct DESERIALIZE(std::tuple<Ts...>) {
    yyjson_val *val;

    void into(std::tuple<Ts...> &tpl) const {
        auto flattened        = flatten(tpl);
        using Tpl             = decltype(flattened);
        constexpr bool is_obj = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;

        yyjson_val *obj = this->val;
        yyjson_val *arr = this->val;
        if (is_obj && !yyjson_is_obj(obj))
            throw type_mismatch_error("object", yyjson_get_type_desc(obj));
        if (!is_obj && !yyjson_is_arr(arr))
            throw type_mismatch_error("array", yyjson_get_type_desc(arr));

        auto   oneofs      = std::array<std::string_view, std::tuple_size_v<Tpl>>{};
        size_t oneof_count = 0;
        size_t idx         = 0;
        tuple_for_each(flattened, [&](auto &item, const size_t) {
            const cpx::TagInfo &t = cpx::json::get_tag_info(item);
            auto               &v = cpx::detail::get_underlying_value(item);
            using T               = std::decay_t<decltype(v)>;

            if (!DESERIALIZABLE(T) || (is_obj && t.key == ""))
                return;

            const size_t i   = idx++;
            yyjson_val  *val = is_obj ? yyjson_obj_getn(obj, t.key.data(), t.key.size()) : yyjson_arr_get(arr, i);
            if (!val && (t.skipmissing || !t.oneof.empty()))
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
                        DESERIALIZE(std::string){val}.into_raw(v);
                    else
                        throw error("field with tag `noserde` can only be deserialized into std::string");
                else {
                    if constexpr (DESERIALIZABLE(T))
                        DESERIALIZE(T){val}.into(v);
                }
            } catch (error &e) {
                if constexpr (is_obj)
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
struct SERIALIZE(std::variant<T...>, std::enable_if_t<(SERIALIZABLE(T) && ...)>) {
    yyjson_mut_doc *doc;

    yyjson_mut_val *from(const std::variant<T...> &v) const {
        return std::visit([&](const auto &var) { return SERIALIZE(std::decay_t<decltype(var)>){doc}.from(var); }, v);
    }
};

template <typename... T>
struct DESERIALIZE(std::variant<T...>, std::enable_if_t<((std::is_default_constructible_v<T> && DESERIALIZABLE(T)) && ...)>) {
    yyjson_val *val;

    void into(std::variant<T...> &v) const {
        bool        done = false;
        std::string type_names;
        (
            [&]() {
                try {
                    if (!done) {
                        auto element = T{};
                        DESERIALIZE(T){val}.into(element);
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
            throw type_mismatch_error(type_names, yyjson_get_type_desc(val));
        }
    }
};

// map
template <typename CT, typename CA, typename T, typename H, typename P, typename A>
struct SERIALIZE(std::unordered_map<std::basic_string<char, CT, CA>, T, H, P, A>, std::enable_if_t<SERIALIZABLE(T)>) {
    yyjson_mut_doc *doc;

    yyjson_mut_val *from(const std::unordered_map<std::basic_string<char, CT, CA>, T, H, P, A> &v) const {
        yyjson_mut_val *obj = yyjson_mut_obj(doc);
        for (auto &[k, v] : v) {
            yyjson_mut_val *key = yyjson_mut_strn(doc, k.data(), k.size()), *item;
            try {
                item = SERIALIZE(T){doc}.from(v);
            } catch (error &e) {
                throw e.add_context(k);
            }
            yyjson_mut_obj_add(obj, key, item);
        }
        return obj;
    }
};

template <typename CT, typename CA, typename T, typename H, typename P, typename A>
struct DESERIALIZE(
    std::unordered_map<std::basic_string<char, CT, CA>, T, H, P, A>,
    std::enable_if_t<std::is_default_constructible_v<T> && DESERIALIZABLE(T)>
) {
    yyjson_val *val;

    void into(std::unordered_map<std::basic_string<char, CT, CA>, T, H, P, A> &v) {
        auto obj = this->val;
        if (!yyjson_is_obj(obj))
            throw type_mismatch_error("object", yyjson_get_type_desc(obj));

        size_t      idx, max;
        yyjson_val *key, *val;
        yyjson_obj_foreach(obj, idx, max, key, val) {
            std::basic_string<char, CT, CA> key_str;
            DESERIALIZE(std::basic_string<char, CT, CA>){key}.into(key_str);
            auto item = T{};
            try {
                DESERIALIZE(T){val}.into(item);
            } catch (error &e) {
                e.add_context(key_str);
                throw;
            }
            v.emplace(std::move(key_str), std::move(item));
        }
    }
};

// reflection
template <typename T>
struct SERIALIZE(T, std::enable_if_t<cpx::json::has_reflect_v<T>>) {
    yyjson_mut_doc *doc;

    yyjson_mut_val *from(const T &v) const {
        return SERIALIZE(cpx::json::const_reflect_t<T>){doc}.from(cpx::json::reflect_of(v));
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<cpx::json::has_reflect_v<T>>) {
    yyjson_val *val;

    void into(T &v) {
        decltype(auto) r = cpx::json::reflect_of(v);
        DESERIALIZE(cpx::json::reflect_t<T>){val}.into(r);
    }
};

// parse and dump
template <typename CT, typename A>
struct PARSE(std::basic_string<char, CT, A>) {
    const std::basic_string<char, CT, A> &src;
    yyjson_read_flag                      flag = YYJSON_READ_NOFLAG;
    const yyjson_alc                     *alc  = nullptr;

    template <typename T>
    void into(T &val, bool src_is_path = false) const {
        yyjson_read_err err;
        try {
            yyjson_doc *doc = src_is_path ? yyjson_read_file(const_cast<char *>(src.c_str()), flag, alc, &err)
                                          : yyjson_read_opts(const_cast<char *>(src.c_str()), src.size(), flag, alc, &err);
            if (!doc)
                throw error(err.msg);

            auto _ = defer([&] { yyjson_doc_free(doc); });
            DESERIALIZE(T){yyjson_doc_get_root(doc)}.into(val);
        } catch (error &err) {
            if (src_is_path)
                err.path = src;
            throw;
        }
    }
};

template <typename CT, typename A>
struct DUMP(std::basic_string<char, CT, A>) {
    yyjson_write_flag flag = YYJSON_WRITE_NOFLAG;
    const yyjson_alc *alc  = nullptr;

    template <typename T>
    std::basic_string<char, CT, A> from(const T &val) const {
        yyjson_mut_doc *doc = yyjson_mut_doc_new(alc);
        if (!doc)
            throw error("Fatal error: cannot create yyjson doc");

        auto _ = defer([&]() { yyjson_mut_doc_free(doc); });
        yyjson_mut_doc_set_root(doc, SERIALIZE(T){doc}.from(val));

        size_t           len;
        yyjson_write_err err;
        const char      *str = yyjson_mut_write_opts(doc, flag, nullptr, &len, &err);
        if (!str)
            throw error(err.msg);

        auto __ = defer([&]() { ::free((void *)str); });
        return {str, len};
    }
};

template <typename T>
void cpx::json::yy_json::parse(const std::string &str, T &val, yyjson_read_flag flag) {
    Parse<std::string>{str, flag}.into(val);
}

template <typename T>
void cpx::json::yy_json::parse_from_file(const std::string &path, T &val, yyjson_read_flag flag) {
    Parse<std::string>{path, flag}.into(val, true);
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T> cpx::json::yy_json::parse(const std::string &str, yyjson_read_flag flag) {
    T val = {};
    Parse<std::string>{str, flag}.into(val);
    return val;
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T>
cpx::json::yy_json::parse_from_file(const std::string &path, yyjson_read_flag flag) {
    T val = {};
    Parse<std::string>{path, flag}.into(val, true);
    return val;
}

template <typename T>
[[nodiscard]]
std::string cpx::json::yy_json::dump(const T &val, yyjson_write_flag flag, const yyjson_alc *alc) {
    return Dump<std::string>{flag, alc}.from(val);
}

#undef SERIALIZE
#undef DESERIALIZE
#undef SERIALIZABLE
#undef DESERIALIZABLE
#undef DUMP
#undef PARSE
#endif
