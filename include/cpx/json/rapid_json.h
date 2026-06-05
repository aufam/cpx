// TODO: implement SAX
#ifndef CPX_JSON_RAPID_JSON_H
#define CPX_JSON_RAPID_JSON_H

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

#ifndef RAPIDJSON_DOCUMENT_H_
#    include <rapidjson/document.h>
#endif

#ifndef RAPIDJSON_WRITER_H_
#    include <rapidjson/writer.h>
#endif

#ifndef RAPIDJSON_READER_H_
#    include <rapidjson/reader.h>
#endif

#ifdef RAPIDJSON_STRINGBUFFER_H_
#    include <rapidjson/stringbuffer.h>
#endif

#ifndef RAPIDJSON_OSTREAMWRAPPER_H_
#    if __has_include(<rapidjson/ostreamwrapper.h>)
#        include <rapidjson/ostreamwrapper.h>
#    endif
#endif

#ifndef RAPIDJSON_ISTREAMWRAPPER_H_
#    if __has_include(<rapidjson/istreamwrapper.h>)
#        include <rapidjson/istreamwrapper.h>
#    endif
#endif

#ifndef RAPIDJSON_ERROR_EN_H_
#    include <rapidjson/error/en.h>
#endif

#define SERIALIZE(...)      cpx::serde::Serialize<rapidjson::Value, __VA_ARGS__>
#define DESERIALIZE(...)    cpx::serde::Deserialize<rapidjson::Value, __VA_ARGS__>
#define SERIALIZABLE(...)   cpx::serde::is_serializable_v<rapidjson::Value, __VA_ARGS__>
#define DESERIALIZABLE(...) cpx::serde::is_deserializable_v<rapidjson::Value, __VA_ARGS__>
#define DUMP(...)           cpx::serde::Dump<rapidjson::Document, __VA_ARGS__>
#define PARSE(...)          cpx::serde::Parse<rapidjson::Document, __VA_ARGS__>

#define SERIALIZE_SAX(OS, ...)    cpx::serde::Serialize<rapidjson::Writer<OS>, __VA_ARGS__>
#define DESERIALIZE_SAX(...)      cpx::serde::Deserialize<rapidjson::Reader, __VA_ARGS__>
#define SERIALIZABLE_SAX(OS, ...) cpx::serde::is_serializable_v<rapidjson::Writer<OS>, __VA_ARGS__>
#define DESERIALIZABLE_SAX(...)   cpx::serde::is_deserializable_v<rapidjson::Reader, __VA_ARGS__>
#define DUMP_SAX(OS, ...)         cpx::serde::Dump<rapidjson::Writer<OS>, __VA_ARGS__>
#define PARSE_SAX(...)            cpx::serde::Parse<rapidjson::Reader, __VA_ARGS__>

namespace cpx::json::rapid_json {
    template <typename From>
    using Serialize = SERIALIZE(From);

    template <typename To>
    using Deserialize = DESERIALIZE(To);

    template <typename From>
    constexpr bool is_serializable_v = SERIALIZABLE(From);

    template <typename To>
    constexpr bool is_deserializable_v = DESERIALIZABLE(To);

    template <typename From>
    using Parse = PARSE(From);

    template <typename To>
    using Dump = DUMP(To);

    template <typename T>
    void parse(const std::string &str, T &val);

    template <typename T>
    void parse(std::istream &, T &val);

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(const std::string &str);

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(std::istream &);

    template <typename T>
    [[nodiscard]]
    std::string dump(const T &val);

    template <typename T>
    void dump(std::ostream &, const T &val);

    template <typename T>
    void dump(const T &&val) = delete;

    template <typename T>
    void dump(std::ostream &, const T &&val) = delete;
} // namespace cpx::json::rapid_json

namespace cpx {
    namespace rapid_json = cpx::json::rapid_json;
}

namespace cpx::json::rapid_json::detail {
    inline std::string type(const rapidjson::Value &val) {
        switch (val.GetType()) {
        case rapidjson::Type::kNullType:
            return "null";
        case rapidjson::Type::kFalseType:
        case rapidjson::Type::kTrueType:
            return "bool";
        case rapidjson::Type::kObjectType:
            return "object";
        case rapidjson::Type::kArrayType:
            return "array";
        case rapidjson::Type::kStringType:
            return "string";
        case rapidjson::Type::kNumberType:
            return "number";
        }
        return "unknown";
    }

    struct Stack {
        enum Type { Null, Bool, Int, Uint, Int64, Uint64, Double, String, Object, Array };
        Type   type;
        Stack *parent   = nullptr;
        bool   nullable = false;

        std::vector<Stack *>                          arr;
        std::unordered_map<std::string_view, Stack *> obj;
    };

    struct Handler {
        using Ch = char;

        virtual bool Null() {
            return false;
        }
        virtual bool Bool(bool) {
            return false;
        }
        virtual bool Int(int) {
            return false;
        }
        virtual bool Uint(unsigned) {
            return false;
        }
        virtual bool Int64(int64_t) {
            return false;
        }
        virtual bool Uint64(uint64_t) {
            return false;
        }
        virtual bool Double(double) {
            return false;
        }
        virtual bool String(const char *, rapidjson::SizeType, bool) {
            return false;
        }
        virtual bool RawNumber(const char *, rapidjson::SizeType, bool) {
            return false;
        }
        virtual bool StartObject() {
            return false;
        }
        virtual bool Key(const char *, rapidjson::SizeType, bool) {
            return false;
        }
        virtual bool EndObject(rapidjson::SizeType) {
            return false;
        }
        virtual bool StartArray() {
            return false;
        }
        virtual bool EndArray(rapidjson::SizeType) {
            return false;
        }
    };

#ifdef RAPIDJSON_ISTREAMWRAPPER_H_
    template <typename T>
    class HandlerFor : public Handler {
    public:
        rapidjson::Reader &reader;
        T                 &v;
        HandlerFor(rapidjson::Reader &reader, T &v)
            : reader(reader)
            , v(v) {}

        // TODO
        void into(T &) {}
    };
#endif
} // namespace cpx::json::rapid_json::detail

// --- DOM ----

// bool & number
template <typename T>
struct SERIALIZE(T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>) {
    rapidjson::Document &doc;

    rapidjson::Value from(T v) const {
        rapidjson::Value val;
        val.Set(v, doc.GetAllocator());
        return val;
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>) {
    const rapidjson::Value &val;

    void into(T &v) {
        if constexpr (std::is_same_v<T, bool>) {
            if (val.IsBool())
                v = val.Get<T>();
            else
                throw cpx::serde::type_mismatch_error("bool", cpx::json::rapid_json::detail::type(val));
        } else if constexpr (std::is_floating_point_v<T>) {
            if (val.IsFloat() || val.IsLosslessFloat() || val.IsDouble() || val.IsLosslessDouble())
                v = val.Get<T>();
            else
                throw cpx::serde::type_mismatch_error("float", cpx::json::rapid_json::detail::type(val));
        } else if constexpr (std::is_unsigned_v<T>) {
            if (val.IsUint() || val.IsUint64())
                v = val.Get<T>();
            else
                throw cpx::serde::type_mismatch_error("uint", cpx::json::rapid_json::detail::type(val));
        } else {
            if (val.IsInt() || val.IsInt64())
                v = val.Get<T>();
            else
                throw cpx::serde::type_mismatch_error("int", cpx::json::rapid_json::detail::type(val));
        }
    }
};

// string
template <typename CT>
struct SERIALIZE(std::basic_string_view<char, CT>) {
    rapidjson::Document &doc;

    rapidjson::Value from(std::basic_string_view<char, CT> v, bool owned = false) const {
        rapidjson::Value val;
        if (owned)
            val.SetString(v.data(), v.size(), doc.GetAllocator());
        else
            val.SetString(rapidjson::StringRef(v.data(), v.size()));
        return val;
    }

    rapidjson::Value from_raw(std::basic_string_view<char, CT> v) const {
        rapidjson::Document doc(&this->doc.GetAllocator());

        constexpr auto flag = rapidjson::kParseDefaultFlags;
        doc.Parse<flag>(v.data(), v.size());
        if (doc.HasParseError())
            throw error(rapidjson::GetParseError_En(doc.GetParseError()));

        return rapidjson::Value(doc, this->doc.GetAllocator());
    }
};

template <typename CT, typename A>
struct SERIALIZE(std::basic_string<char, CT, A>) {
    rapidjson::Document &doc;

    rapidjson::Value from(const std::basic_string<char, CT, A> &v) const {
        return SERIALIZE(std::basic_string_view<char, CT>){doc}.from(v, false);
    }

    rapidjson::Value from(std::basic_string<char, CT, A> &&v) const {
        return SERIALIZE(std::basic_string_view<char, CT>){doc}.from(v, true);
    }

    rapidjson::Value from_raw(const std::basic_string<char, CT, A> &v) const {
        return SERIALIZE(std::basic_string_view<char, CT>){doc}.from_raw(v);
    }
};

template <typename CT, typename A>
struct DESERIALIZE(std::basic_string<char, CT, A>) {
    const rapidjson::Value &val;

    void into(std::basic_string<char, CT, A> &v) {
        if (val.IsString())
            v = std::basic_string<char, CT, A>(val.GetString(), val.GetStringLength());
        else
            throw cpx::serde::type_mismatch_error("string", cpx::json::rapid_json::detail::type(val));
    }

    void into_raw(std::string &v) {
        rapidjson::StringBuffer                    buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        val.Accept(writer);
        v = std::basic_string<char, CT, A>(buf.GetString(), buf.GetLength());
    }
};

// optional
template <typename T>
struct SERIALIZE(std::optional<T>, std::enable_if_t<SERIALIZABLE(T)>) {
    rapidjson::Document &doc;

    rapidjson::Value from(const std::optional<T> &v) const {
        if (v.has_value())
            return SERIALIZE(T){doc}.from(*v);

        return rapidjson::Value(rapidjson::Type::kNullType);
    }
};

template <typename T>
struct DESERIALIZE(std::optional<T>, std::enable_if_t<std::is_default_constructible_v<T> && DESERIALIZABLE(T)>) {
    const rapidjson::Value &val;

    void into(std::optional<T> &v) {
        if (val.IsNull())
            v = std::nullopt;
        else {
            v = T{};
            DESERIALIZE(T){val}.into(*v);
        }
    }
};

// variant
template <typename... T>
struct SERIALIZE(std::variant<T...>, std::enable_if_t<(SERIALIZABLE(T) && ...)>) {
    rapidjson::Document &doc;

    rapidjson::Value from(const std::variant<T...> &v) const {
        return std::visit([this](const auto &var) { return SERIALIZE(std::decay_t<decltype(var)>){doc}.from(var); }, v);
    }
};

template <typename... T>
struct DESERIALIZE(std::variant<T...>, std::enable_if_t<((std::is_default_constructible_v<T> && DESERIALIZABLE(T)) && ...)>) {
    const rapidjson::Value &val;

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
            ...
        );
        if (!done) {
            type_names.pop_back();
            throw type_mismatch_error(type_names, cpx::json::rapid_json::detail::type(val));
        }
    }
};

// array
template <typename T, size_t N>
struct SERIALIZE(std::array<T, N>, std::enable_if_t<SERIALIZABLE(T)>) {
    rapidjson::Document &doc;

    template <typename Container>
    rapidjson::Value from_container(const Container &v) const {
        rapidjson::Value arr(rapidjson::Type::kArrayType);
        arr.Reserve(v.size(), doc.GetAllocator());
        for (auto &item : v)
            arr.PushBack(SERIALIZE(T){doc}.from(item), doc.GetAllocator());
        return arr;
    }

    rapidjson::Value from(const std::array<T, N> &v) const {
        return from_container(v);
    }
};

template <typename T, size_t N>
struct DESERIALIZE(std::array<T, N>, std::enable_if_t<DESERIALIZABLE(T)>) {
    const rapidjson::Value &val;

    template <typename Container, typename F>
    void into_container(Container &v, F &&on_mismatch) const {
        if (!val.IsArray())
            throw type_mismatch_error("array", cpx::json::rapid_json::detail::type(val));

        const auto  &arr = val.GetArray();
        const size_t n   = arr.Size();
        if (n != v.size())
            on_mismatch(n);

        for (size_t i = 0; i < n; ++i)
            try {
                DESERIALIZE(T){arr[i]}.into(v[i]);
            } catch (error &e) {
                e.add_context(i);
                throw;
            }
    }

    void into(std::array<T, N> &v) const {
        into_container(v, [](size_t got) { throw size_mismatch_error(N, got); });
    };
};

template <typename T, typename A>
struct SERIALIZE(std::vector<T, A>, std::enable_if_t<SERIALIZABLE(T)>) {
    rapidjson::Document &doc;

    rapidjson::Value from(const std::vector<T, A> &v) const {
        return SERIALIZE(std::array<T, 1>){doc}.from_container(v);
    }
};

template <typename T, typename A>
struct DESERIALIZE(std::vector<T, A>, std::enable_if_t<DESERIALIZABLE(T) && std::is_default_constructible_v<T>>) {
    const rapidjson::Value &val;

    void into(std::vector<T, A> &v) const {
        DESERIALIZE(std::array<T, 1>){val}.into_container(v, [&v](size_t got) { v.resize(got); });
    }
};

// object
template <typename CT, typename CA, typename T, typename H, typename P, typename A>
struct SERIALIZE(std::unordered_map<std::basic_string<char, CT, CA>, T, H, P, A>, std::enable_if_t<SERIALIZABLE(T)>) {
    rapidjson::Document &doc;

    rapidjson::Value from(const std::unordered_map<std::basic_string<char, CT, CA>, T, H, P, A> &v) const {
        rapidjson::Value obj(rapidjson::Type::kObjectType);
        obj.MemberReserve(v.size(), doc.GetAllocator());
        for (auto &[key, item] : v)
            obj.AddMember(
                SERIALIZE(std::basic_string<char, CT, CA>){doc}.from(key), SERIALIZE(T){doc}.from(item), doc.GetAllocator()
            );
        return obj;
    }
};

template <typename CT, typename CA, typename T, typename H, typename P, typename A>
struct
    DESERIALIZE(std::unordered_map<std::basic_string<char, CT, CA>, T, H, P, A>, std::enable_if_t<std::is_default_constructible_v<T> && DESERIALIZABLE(T)>) {
    const rapidjson::Value &val;

    void into(std::unordered_map<std::basic_string<char, CT, CA>, T, H, P, A> &v) const {
        if (!val.IsObject())
            throw type_mismatch_error("object", cpx::json::rapid_json::detail::type(val));

        const auto obj = val.GetObject();
        v.reserve(obj.MemberCount());
        for (auto &[key, val] : obj) {
            auto k    = std::string();
            auto item = T{};
            try {
                DESERIALIZE(std::basic_string<char, CT, CA>){key}.into(k);
                DESERIALIZE(T){val}.into(item);
            } catch (error &e) {
                e.add_context(std::string_view(key.GetString(), key.GetStringLength()));
                throw;
            }
            v.emplace(std::move(k), std::move(item));
        }
    }
};

// tuple
template <typename... Ts>
struct SERIALIZE(std::tuple<Ts...>) {
    rapidjson::Document &doc;

    rapidjson::Value from(const std::tuple<Ts...> &tpl) {
        auto flatten            = cpx::flatten(tpl);
        using Tpl               = decltype(flatten);
        constexpr bool   is_obj = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;
        rapidjson::Value val(is_obj ? rapidjson::kObjectType : rapidjson::kArrayType);

        is_obj ? val.MemberReserve(std::tuple_size_v<Tpl>, doc.GetAllocator())
               : val.Reserve(std::tuple_size_v<Tpl>, doc.GetAllocator());

        auto   oneofs      = std::array<std::string_view, std::tuple_size_v<Tpl>>{};
        size_t oneof_count = 0;
        size_t idx         = 0;
        tuple_for_each(flatten, [&](auto &item, const size_t) {
            const cpx::TagInfo &t       = cpx::json::get_tag_info(item);
            auto               &v       = cpx::detail::get_underlying_value(item);
            using T                     = std::decay_t<decltype(v)>;
            constexpr bool serializable = SERIALIZABLE(T);

            if (!serializable || (is_obj && t.key == ""))
                return;

            size_t i = idx++;
            if ((t.omitempty || !t.oneof.empty()) && detail::is_empty_value(v) && is_obj)
                return;

            if (!t.oneof.empty()) {
                for (size_t j = 0; j < oneof_count; ++j) {
                    if (oneofs[j] == t.oneof)
                        throw duplicate_oneof_error(t.oneof);
                }
                oneofs[oneof_count++] = t.oneof;
            }

            rapidjson::Value sub;
            try {
                if (t.noserde)
                    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
                        sub = SERIALIZE(std::string_view){doc}.from_raw(v);
                    else
                        throw error("field with tag `noserde` can only be serialized from std::string");
                else {
                    if constexpr (serializable)
                        sub = SERIALIZE(T){doc}.from(v);
                }
            } catch (error &e) {
                if constexpr (is_obj)
                    e.add_context(t.key);
                else
                    e.add_context(i);
                throw;
            }
            if constexpr (is_obj) {
                auto obj = val.GetObject();
                obj.AddMember(rapidjson::StringRef(t.key.data(), t.key.size()), std::move(sub), doc.GetAllocator());
            } else {
                auto arr = val.GetArray();
                arr.PushBack(std::move(sub), doc.GetAllocator());
            }
        });

        return val;
    }
};

template <typename... Ts>
struct DESERIALIZE(std::tuple<Ts...>) {
    const rapidjson::Value &val;

    void into(std::tuple<Ts...> &tpl) const {
        auto flattened        = flatten(tpl);
        using Tpl             = decltype(flattened);
        constexpr bool is_obj = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;

        if (!is_obj && !val.IsArray())
            throw type_mismatch_error("array", cpx::json::rapid_json::detail::type(val));
        if (is_obj && !val.IsObject())
            throw type_mismatch_error("table", cpx::json::rapid_json::detail::type(val));

        auto   oneofs      = std::array<std::string_view, std::tuple_size_v<Tpl>>{};
        size_t oneof_count = 0;
        size_t idx         = 0;
        tuple_for_each(flattened, [&](auto &item, const size_t) {
            const cpx::TagInfo &t         = cpx::json::get_tag_info(item);
            auto               &v         = cpx::detail::get_underlying_value(item);
            using T                       = std::decay_t<decltype(v)>;
            constexpr bool deserializable = DESERIALIZABLE(T);

            if (!deserializable || (is_obj && t.key == ""))
                return;

            const size_t            i = idx++;
            const rapidjson::Value  empty(rapidjson::Type::kNullType);
            const rapidjson::Value *ptr = &empty;
            if constexpr (is_obj) {
                rapidjson::Value key;
                key.SetString(rapidjson::StringRef(t.key.data(), t.key.size()));

                auto obj = val.GetObject();
                if (auto it = obj.FindMember(key); it != obj.end())
                    ptr = &it->value;
            } else {
                auto arr = val.GetArray();
                if (i < arr.Size())
                    ptr = &arr[i];
            }
            if (ptr == &empty && (t.skipmissing || !t.oneof.empty()))
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
                        DESERIALIZE(std::string){*ptr}.into_raw(v);
                    else
                        throw error("field with tag `noserde` can only be deserialized into std::string");
                else {
                    if constexpr (DESERIALIZABLE(T))
                        DESERIALIZE(T){*ptr}.into(v);
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

// generic reflection
template <typename T>
struct SERIALIZE(T, std::enable_if_t<cpx::json::has_reflect_v<T>>) {
    rapidjson::Document &doc;

    rapidjson::Value from(const T &v) const {
        return SERIALIZE(cpx::json::const_reflect_t<T>){doc}.from(cpx::json::reflect_of(v));
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<cpx::json::has_reflect_v<T>>) {
    const rapidjson::Value &val;

    void into(T &v) const {
        decltype(auto) r = cpx::json::reflect_of(v);
        DESERIALIZE(cpx::json::reflect_t<T>){val}.into(r);
    }
};

// --- SAX ----

// bool and number
template <typename OS, typename T>
struct SERIALIZE_SAX(OS, T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>) {
    rapidjson::Writer<OS> &writer;

    void from(T v) const {
        bool res;
        if constexpr (std::is_same_v<T, bool>) {
            res = writer.Bool(v);
        } else if constexpr (std::is_floating_point_v<T>) {
            res = writer.Double(v);
        } else if constexpr (std::is_unsigned_v<T>) {
            res = writer.Uint64(v);
        } else {
            res = writer.Int64(v);
        }
        if (!res) {
            throw error("error"); // TODO
        }
    }
};

template <typename T>
struct cpx::serde::Deserialize<rapidjson::Reader, T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>>
    : public cpx::json::rapid_json::detail::HandlerFor<T> {
    using cpx::json::rapid_json::detail::HandlerFor<T>::HandlerFor;
    using cpx::json::rapid_json::detail::HandlerFor<T>::v;

    bool Bool(bool b) override {
        if constexpr (std::is_same_v<T, bool>) {
            v = b;
            return true;
        } else {
            return false;
        }
    }

    bool Int(int i) override {
        if constexpr (!std::is_same_v<T, bool> && std::is_integral_v<T>) {
            v = T(i);
            return true;
        } else {
            return false;
        }
    }

    bool Uint(unsigned u) override {
        if constexpr (!std::is_same_v<T, bool> && std::is_unsigned_v<T>) {
            v = T(u);
            return true;
        } else {
            return false;
        }
    }

    bool Int64(int64_t i) override {
        if constexpr (std::is_same_v<T, int64_t>) {
            v = i;
            return true;
        } else {
            return false;
        }
    }

    bool Uint64(uint64_t u) override {
        if constexpr (std::is_same_v<T, uint64_t>) {
            v = u;
            return true;
        } else {
            return false;
        }
    }

    bool Double(double d) override {
        if constexpr (std::is_floating_point_v<T>) {
            v = T(d);
            return true;
        } else {
            return false;
        }
    }
};

// string
template <typename OS, typename CT>
struct SERIALIZE_SAX(OS, std::basic_string_view<char, CT>) {
    rapidjson::Writer<OS> &writer;

    void from(const std::basic_string_view<char, CT> &v) const {
        if (!writer.String(v.data(), v.size()))
            throw error("error"); // TODO
    }

    void from_raw(const std::basic_string_view<char, CT> &v) const {
        rapidjson::Document doc;
        doc.Parse(v.data(), v.size());
        if (doc.HasParseError())
            throw error(rapidjson::GetParseError_En(doc.GetParseError()));
        if (!writer.RawValue(v.data(), v.size(), doc.GetType()))
            throw error("error"); // TODO
    }
};

template <typename OS, typename CT, typename A>
struct SERIALIZE_SAX(OS, std::basic_string<char, CT, A>) {
    rapidjson::Writer<OS> &writer;

    void from(const std::basic_string<char, CT, A> &v) const {
        Serialize<rapidjson::Writer<OS>, std::basic_string_view<char, CT>>{writer}.from(v);
    }

    void from_raw(const std::basic_string<char, CT, A> &v) const {
        Serialize<rapidjson::Writer<OS>, std::basic_string_view<char, CT>>{writer}.from_raw(v);
    }
};

template <typename CT, typename A>
struct cpx::serde::Deserialize<rapidjson::Reader, std::basic_string<char, CT, A>>
    : public cpx::json::rapid_json::detail::HandlerFor<std::basic_string<char, CT, A>> {
    using cpx::json::rapid_json::detail::HandlerFor<std::basic_string<char, CT, A>>::HandlerFor;

    bool String(const char *str, rapidjson::SizeType len, bool) override {
        this->v = std::basic_string<char, CT, A>(str, len);
        return true;
    }
};

// optional
template <typename OS, typename T>
struct SERIALIZE_SAX(OS, std::optional<T>, std::enable_if_t<SERIALIZABLE_SAX(OS, T)>) {
    rapidjson::Writer<OS> &writer;

    void from(const std::optional<T> &v) const {
        if (v.has_value())
            Serialize<rapidjson::Writer<OS>, T>{writer}.from(*v);
        else
            writer.Null();
    }
};

// array
template <typename OS, typename T, size_t N>
struct SERIALIZE_SAX(OS, std::array<T, N>, std::enable_if_t<SERIALIZABLE_SAX(OS, T)>) {
    rapidjson::Writer<OS> &writer;

    template <typename Container>
    void from_container(const Container &v) const {
        writer.StartArray();
        for (auto &item : v)
            Serialize<rapidjson::Writer<OS>, T>{writer}.from(item);
        writer.EndArray(v.size());
    }

    void from(const std::array<T, N> &v) const {
        from_container(v);
    }
};

template <typename OS, typename T>
struct SERIALIZE_SAX(OS, std::vector<T>, std::enable_if_t<SERIALIZABLE_SAX(OS, T)>) {
    rapidjson::Writer<OS> &writer;

    void from(const std::vector<T> &v) const {
        SERIALIZE_SAX(OS, std::array<T, 1>){writer}.from_container(v);
    }
};

// map
template <typename OS, typename CT, typename CA, typename T, typename H, typename P, typename A>
struct
    SERIALIZE_SAX(OS, std::unordered_map<std::basic_string<char, CT, CA>, T, H, P, A>, std::enable_if_t<SERIALIZABLE_SAX(OS, T)>) {
    rapidjson::Writer<OS> &writer;

    void from(const std::unordered_map<std::basic_string<char, CT, CA>, T, H, P, A> &v) const {
        writer.StartObject();
        for (auto &[k, v] : v) {
            writer.Key(k.c_str(), k.size());
            Serialize<rapidjson::Writer<OS>, T>{writer}.from(v);
        }
        writer.EndObject();
    }
};

// tuple
template <typename OS, typename... Ts>
struct SERIALIZE_SAX(OS, std::tuple<Ts...>) {
    rapidjson::Writer<OS> &writer;

    void from(const std::tuple<Ts...> &tpl) {
        auto flatten          = cpx::flatten(tpl);
        using Tpl             = decltype(flatten);
        constexpr bool is_obj = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;

        if constexpr (is_obj)
            writer.StartObject();
        else
            writer.StartArray();

        size_t idx = 0;
        tuple_for_each(flatten, [&](auto &item, const size_t) {
            const cpx::TagInfo &t       = cpx::json::get_tag_info(item);
            auto               &v       = cpx::detail::get_underlying_value(item);
            using T                     = std::decay_t<decltype(v)>;
            constexpr bool serializable = SERIALIZABLE_SAX(OS, T);

            if (!serializable || (is_obj && t.key == ""))
                return;

            size_t i = idx++;
            if (t.omitempty && detail::is_empty_value(v) && is_obj)
                return;

            try {
                if (t.noserde)
                    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
                        SERIALIZE_SAX(OS, std::string_view){writer}.from_raw(v);
                    else
                        throw error("field with tag `noserde` can only be serialized from std::string");
                else {
                    if constexpr (serializable) {
                        if constexpr (is_obj) {
                            writer.Key(t.key.data(), t.key.size());
                            SERIALIZE_SAX(OS, T){writer}.from(v);
                        } else
                            SERIALIZE_SAX(OS, T){writer}.from(v);
                    }
                }
            } catch (error &e) {
                if constexpr (is_obj)
                    e.add_context(t.key);
                else
                    e.add_context(i);
                throw;
            }
        });

        if constexpr (is_obj)
            writer.EndObject(idx);
        else
            writer.EndArray(idx);
    }
};

// reflect
template <typename OS, typename T>
struct SERIALIZE_SAX(OS, T, std::enable_if_t<cpx::json::has_reflect_v<T>>) {
    rapidjson::Writer<OS> &writer;

    void from(const T &v) const {
        return SERIALIZE_SAX(OS, cpx::json::const_reflect_t<T>){writer}.from(cpx::json::reflect_of(v));
    }
};

// --- parse and dump ---
template <typename CT, typename A>
struct PARSE(std::basic_string<char, CT, A>) {
    const std::basic_string<char, CT, A> &src;

    template <typename T>
    void into(T &v) const {
        rapidjson::Document doc;

        constexpr auto flag = rapidjson::kParseDefaultFlags;
        doc.Parse<flag>(src.c_str(), src.size());
        if (doc.HasParseError())
            throw error(rapidjson::GetParseError_En(doc.GetParseError()));

        const rapidjson::Value &val = doc;
        DESERIALIZE(T){val}.into(v);
    }
};

#ifdef RAPIDJSON_ISTREAMWRAPPER_H_
template <>
struct cpx::serde::Parse<rapidjson::Reader, std ::istream> {
    std::istream &is;

    template <typename T>
    std::istream &into(T &v) const {
        rapidjson::IStreamWrapper isw(is);
        rapidjson::Reader         reader;

        Deserialize<rapidjson::Reader, T> handler(reader, v);
        if (!reader.Parse(isw, handler))
            throw error(rapidjson::GetParseError_En(reader.GetParseErrorCode()));

        return is;
    }

    template <typename T>
    std::istream &operator>>(T &v) const {
        return into(v);
    }
};
#endif

template <typename CT, typename A>
struct DUMP(std::basic_string<char, CT, A>) {

    template <typename T>
    std::basic_string<char, CT, A> from(const T &val) const {
        rapidjson::Document doc;
        rapidjson::Value    v = Serialize<rapidjson::Value, T>{doc}.from(val);

        rapidjson::StringBuffer                    buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);

        v.Accept(writer);
        return std::basic_string<char, CT, A>(buf.GetString(), buf.GetSize());
    }
};

#ifdef RAPIDJSON_OSTREAMWRAPPER_H_
template <>
struct DUMP_SAX(rapidjson::OStreamWrapper, std::ostream) {
    std::ostream &os;

    template <typename T>
    std::ostream &from(const T &val) const {
        rapidjson::OStreamWrapper                    osw(os);
        rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
        Serialize<rapidjson::Writer<rapidjson::OStreamWrapper>, T>{writer}.from(val);
        return os;
    }

    template <typename T>
    std::ostream &operator<<(const T &val) const {
        return from(val);
    }
};
#endif

template <typename T>
void cpx::json::rapid_json::parse(const std::string &str, T &val) {
    Parse<std::string>{str}.into(val);
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T> cpx::json::rapid_json::parse(const std::string &str) {
    T val = {};
    Parse<std::string>{str}.into(val);
    return val;
}

#ifdef RAPIDJSON_ISTREAMWRAPPER_H_
template <typename T>
void cpx::json::rapid_json::parse(std::istream &is, T &val) {
    cpx::serde::Parse<rapidjson::Reader, std::istream>{is}.into(val);
}
#endif

template <typename T>
[[nodiscard]]
std::string cpx::json::rapid_json::dump(const T &val) {
    return Dump<std::string>{}.from(val);
}

#ifdef RAPIDJSON_OSTREAMWRAPPER_H_
template <typename T>
void cpx::json::rapid_json::dump(std::ostream &os, const T &val) {
    cpx::serde::Dump<rapidjson::Writer<rapidjson::OStreamWrapper>, std::ostream>{os}.from(val);
}
#endif

namespace cpx::json::rapid_json {
    constexpr struct IO {
#ifdef RAPIDJSON_OSTREAMWRAPPER_H_
        friend cpx::serde::Dump<rapidjson::Writer<rapidjson::OStreamWrapper>, std::ostream>
        operator<<(std::ostream &os, const IO &) {
            return {os};
        }
#endif

        friend cpx::serde::Parse<rapidjson::Reader, std::istream> operator>>(std::istream &is, const IO &) {
            return {is};
        }
    } io;
} // namespace cpx::json::rapid_json

#undef SERIALIZE
#undef DESERIALIZE
#undef SERIALIZABLE
#undef DESERIALIZABLE
#undef PARSE
#undef DUMP
#undef SERIALIZE_SAX
#undef DESERIALIZE_SAX
#undef SERIALIZABLE_SAX
#undef DESERIALIZABLE_SAX
#undef PARSE_SAX
#undef DUMP_SAX
#endif
