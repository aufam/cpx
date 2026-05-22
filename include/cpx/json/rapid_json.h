// TODO: implement SAX
#ifndef CPX_JSON_RAPID_JSON_H
#define CPX_JSON_RAPID_JSON_H

#include <cpx/json/json.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/reflect.h>
#include <cpx/extend.h>
#include <cpx/defer.h>
#include <array>
#include <variant>
#include <vector>
#include <tuple>
#include <unordered_map>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/error/en.h>

namespace cpx::json::rapid_json {
    template <typename From>
    using Serialize = ::cpx::serde::Serialize<rapidjson::Value, From>;

    template <typename To>
    using Deserialize = ::cpx::serde::Deserialize<rapidjson::Value, To>;

    template <typename From>
    using Parse = ::cpx::serde::Parse<rapidjson::Document, From>;

    template <typename To>
    using Dump = ::cpx::serde::Dump<rapidjson::Document, To>;

    template <typename T>
    void parse(const std::string &str, T &val);

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(const std::string &str);

    template <typename T>
    [[nodiscard]]
    std::string dump(const T &val);
} // namespace cpx::json::rapid_json

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

    template <typename T>
    struct is_std_string : std::false_type {};

    template <typename CT, typename A>
    struct is_std_string<std::basic_string<char, CT, A>> : std::false_type {};

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

    template <typename T>
    class HandlerFor : Handler {
    public:
        rapidjson::Reader &reader;
        T                 &v;
        HandlerFor(rapidjson::Reader &reader, T &v)
            : reader(reader)
            , v(v) {}

        // TODO
        void into(T &) {}
    };
} // namespace cpx::json::rapid_json::detail

template <typename CT, typename A>
struct cpx::serde::Parse<rapidjson::Document, std::basic_string<char, CT, A>> {
    const std::basic_string<char, CT, A> &src;

    template <typename T>
    void into(T &v) const {
        rapidjson::Document doc;

        constexpr auto flag = rapidjson::kParseDefaultFlags;
        doc.Parse<flag>(src.c_str(), src.size());
        if (doc.HasParseError())
            throw error(rapidjson::GetParseError_En(doc.GetParseError()));

        const rapidjson::Value &val = doc;
        Deserialize<rapidjson::Value, T>{val}.into(v);
    }
};

template <>
struct cpx::serde::Parse<rapidjson::Reader, std::istream> {
    std::istream &ist;

    template <typename T>
    void into(T &v) const {
        rapidjson::IStreamWrapper isw(ist);
        rapidjson::Reader         reader;

        Deserialize<rapidjson::Reader, T> handler(reader, v);
        if (!reader.Parse(isw, handler))
            throw error(rapidjson::GetParseError_En(reader.GetParseErrorCode()));
    }
};

template <typename CT, typename A>
struct cpx::serde::Dump<rapidjson::Document, std::basic_string<char, CT, A>> {

    template <typename T>
    std::basic_string<char, CT, A> from(const T &val) const {
        rapidjson::Document doc;
        rapidjson::Value    v = Serialize<rapidjson::Value, T>{doc}.from(val);

        rapidjson::StringBuffer                    buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);

        v.Accept(writer);
        return buf.GetString();
    }
};

template <>
struct cpx::serde::Dump<rapidjson::Writer<rapidjson::OStreamWrapper>, std::ostream> {
    std::ostream &os;

    template <typename T>
    std::ostream &from(const T &val) const {
        rapidjson::OStreamWrapper                    osw(os);
        rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
        Serialize<rapidjson::Writer<rapidjson::OStreamWrapper>, T>{writer}.from(val);
        return os;
    }
};

// bool & number
template <typename T>
struct cpx::serde::Serialize<rapidjson::Value, T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>> {
    rapidjson::Document &doc;

    rapidjson::Value from(T v) const {
        rapidjson::Value val;
        val.Set(v, doc.GetAllocator());
        return val;
    }
};


template <typename T>
struct cpx::serde::Deserialize<rapidjson::Value, T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>> {
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
template <typename CT, typename A>
struct cpx::serde::Serialize<rapidjson::Value, std::basic_string<char, CT, A>> {
    rapidjson::Document &doc;

    rapidjson::Value from(const std::basic_string<char, CT, A> &v) const {
        rapidjson::Value val;
        val.SetString(v.data(), v.size(), doc.GetAllocator());
        return val;
    }

    rapidjson::Value from_raw(const std::basic_string<char, CT, A> &v) const {
        rapidjson::Document doc(&this->doc.GetAllocator());

        constexpr auto flag = rapidjson::kParseDefaultFlags;
        doc.Parse<flag>(v.c_str(), v.size());
        if (doc.HasParseError())
            throw error(rapidjson::GetParseError_En(doc.GetParseError()));

        return rapidjson::Value(doc, this->doc.GetAllocator());
    }
};

template <typename CT>
struct cpx::serde::Serialize<rapidjson::Value, std::basic_string_view<char, CT>> {
    rapidjson::Document &doc;

    rapidjson::Value from(std::basic_string_view<char, CT> v) const {
        rapidjson::Value val;
        val.SetString(v.data(), v.size(), doc.GetAllocator());
        return val;
    }
};

template <typename CT, typename A>
struct cpx::serde::Deserialize<rapidjson::Value, std::basic_string<char, CT, A>> {
    const rapidjson::Value &val;

    void into(std::basic_string<char, CT, A> &v) {
        if (val.IsString())
            v = val.GetString();
        else
            throw cpx::serde::type_mismatch_error("string", cpx::json::rapid_json::detail::type(val));
    }

    void into_raw(std::string &v) {
        rapidjson::StringBuffer                    buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        val.Accept(writer);
        v = buf.GetString();
    }
};

// optional
template <typename T>
struct cpx::serde::
    Serialize<rapidjson::Value, std::optional<T>, std::enable_if_t<cpx::serde::is_serializable_v<rapidjson::Value, T>>> {
    rapidjson::Document &doc;

    rapidjson::Value from(const std::optional<T> &v) const {
        if (v.has_value())
            return Serialize<rapidjson::Value, T>{doc}.from(*v);

        return rapidjson::Value(rapidjson::Type::kNullType);
    }
};

template <typename T>
struct cpx::serde::Deserialize<
    rapidjson::Value,
    std::optional<T>,
    std::enable_if_t<std::is_default_constructible_v<T> && cpx::serde::is_deserializable_v<rapidjson::Value, T>>> {
    const rapidjson::Value &val;

    void into(std::optional<T> &v) {
        if (val.IsNull())
            v = std::nullopt;
        else {
            v = T{};
            Deserialize<rapidjson::Value, T>{val}.into(*v);
        }
    }
};

// variant
template <typename... T>
struct cpx::serde::Serialize<
    rapidjson::Value,
    std::variant<T...>,
    std::enable_if_t<(cpx::serde::is_serializable_v<rapidjson::Value, T> && ...)>> {
    rapidjson::Document &doc;

    rapidjson::Value from(const std::variant<T...> &v) const {
        return std::visit(
            [this](const auto &var) { return Serialize<rapidjson::Value, std::decay_t<decltype(var)>>{doc}.from(var); }, v
        );
    }
};

template <typename... T>
struct cpx::serde::Deserialize<
    rapidjson::Value,
    std::variant<T...>,
    std::enable_if_t<((std::is_default_constructible_v<T> && cpx::serde::is_deserializable_v<rapidjson::Value, T>) && ...)>> {
    const rapidjson::Value &val;

    void into(std::variant<T...> &v) const {
        bool        done = false;
        std::string type_names;
        (
            [&]() {
                try {
                    if (!done) {
                        auto element = T{};
                        Deserialize<rapidjson::Value, T>{val}.into(element);
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
struct cpx::serde::
    Serialize<rapidjson::Value, std::array<T, N>, std::enable_if_t<cpx::serde::is_serializable_v<rapidjson::Value, T>>> {
    rapidjson::Document &doc;

    rapidjson::Value from(const std::array<T, N> &v) const {
        rapidjson::Value arr(rapidjson::Type::kArrayType);
        for (auto &item : v)
            arr.PushBack(Serialize<rapidjson::Value, T>{doc}.from(item), doc.GetAllocator());
        return arr;
    }
};

template <typename T, size_t N>
struct cpx::serde::
    Deserialize<rapidjson::Value, std::array<T, N>, std::enable_if_t<cpx::serde::is_deserializable_v<rapidjson::Value, T>>> {
    const rapidjson::Value &val;

    void into(std::array<T, N> &v) const {
        if (!val.IsArray())
            throw type_mismatch_error("array", cpx::json::rapid_json::detail::type(val));

        const auto  &arr = val.GetArray();
        const size_t n   = arr.Size();
        if (n != N)
            throw size_mismatch_error(N, n);

        for (size_t i = 0; i < n; ++i)
            try {
                Deserialize<rapidjson::Value, T>{arr[i]}.into(v[i]);
            } catch (error &e) {
                e.add_context(i);
                throw;
            }
    };
};

template <typename T, typename A>
struct cpx::serde::
    Serialize<rapidjson::Value, std::vector<T, A>, std::enable_if_t<cpx::serde::is_serializable_v<rapidjson::Value, T>>> {
    rapidjson::Document &doc;

    rapidjson::Value from(const std::vector<T, A> &v) const {
        rapidjson::Value arr(rapidjson::Type::kArrayType);
        for (auto &item : v)
            arr.PushBack(Serialize<rapidjson::Value, T>{doc}.from(item), doc.GetAllocator());
        return arr;
    }
};

template <typename T, typename A>
struct cpx::serde::Deserialize<
    rapidjson::Value,
    std::vector<T, A>,
    std::enable_if_t<cpx::serde::is_deserializable_v<rapidjson::Value, T> && std::is_default_constructible_v<T>>> {
    const rapidjson::Value &val;

    void into(std::vector<T, A> &v) const {
        if (!val.IsArray())
            throw type_mismatch_error("array", cpx::json::rapid_json::detail::type(val));

        const auto  &arr = val.GetArray();
        const size_t n   = arr.Size();
        v.resize(n);

        for (size_t i = 0; i < n; ++i)
            try {
                Deserialize<rapidjson::Value, T>{arr[i]}.into(v[i]);
            } catch (error &e) {
                e.add_context(i);
                throw;
            }
    };
};

// object
template <typename CT, typename CA, typename T, typename H, typename P, typename A>
struct cpx::serde::Serialize<
    rapidjson::Value,
    std::unordered_map<std::basic_string<char, CT, CA>, T, H, P, A>,
    std::enable_if_t<cpx::serde::is_serializable_v<rapidjson::Value, T>>> {
    rapidjson::Document &doc;

    rapidjson::Value from(const std::unordered_map<std::basic_string<char, CT, CA>, T, H, P, A> &v) const {
        rapidjson::Value obj(rapidjson::Type::kObjectType);
        for (auto &[key, item] : v)
            obj.AddMember(
                Serialize<rapidjson::Value, std::string>{doc}.from(key),
                Serialize<rapidjson::Value, T>{doc}.from(item),
                doc.GetAllocator()
            );
        return obj;
    }
};

template <typename CT, typename CA, typename T, typename H, typename P, typename A>
struct cpx::serde::Deserialize<
    rapidjson::Value,
    std::unordered_map<std::basic_string<char, CT, CA>, T, H, P, A>,
    std::enable_if_t<std::is_default_constructible_v<T> && cpx::serde::is_deserializable_v<rapidjson::Value, T>>> {
    const rapidjson::Value &val;

    void into(std::unordered_map<std::basic_string<char, CT, CA>, T, H, P, A> &v) const {
        if (!val.IsObject())
            throw type_mismatch_error("object", cpx::json::rapid_json::detail::type(val));

        for (auto &[key, val] : val.GetObject()) {
            auto k    = std::string();
            auto item = T{};
            try {
                Deserialize<rapidjson::Value, std::string>{key}.into(k);
                Deserialize<rapidjson::Value, T>{val}.into(item);
            } catch (error &e) {
                e.add_context(key.GetString());
                throw;
            }
            v.emplace(std::move(k), std::move(item));
        }
    }
};

// tuple
template <typename... Ts>
struct cpx::serde::Serialize<rapidjson::Value, std::tuple<Ts...>> {
    rapidjson::Document &doc;

    rapidjson::Value from(const std::tuple<Ts...> &tpl) {
        auto flatten            = cpx::flatten(tpl);
        using Tpl               = decltype(flatten);
        constexpr bool   is_obj = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;
        rapidjson::Value val(is_obj ? rapidjson::kObjectType : rapidjson::kArrayType);

        size_t idx = 0;
        tuple_for_each(tpl, [&](auto &item, const size_t) {
            const cpx::TagInfo &t       = cpx::json::get_tag_info(item);
            auto               &v       = cpx::detail::get_underlying_value(item);
            using T                     = std::decay_t<decltype(v)>;
            constexpr bool serializable = cpx::serde::is_serializable_v<rapidjson::Value, T>;

            if (!serializable || (is_obj && t.key == ""))
                return;

            size_t i = idx++;
            if (t.omitempty && detail::is_empty_value(v) && is_obj)
                return;

            rapidjson::Value sub;
            try {
                if (t.noserde)
                    if constexpr (std::is_same_v<T, std::string>)
                        sub = Serialize<rapidjson::Value, std::string>{doc}.from_raw(v);
                    else
                        throw error("field with tag `noserde` can only be serialized from std::string");
                else {
                    if constexpr (serializable)
                        sub = Serialize<rapidjson::Value, T>{doc}.from(v);
                }
            } catch (error &e) {
                if (is_obj)
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
struct cpx::serde::Deserialize<rapidjson::Value, std::tuple<Ts...>> {
    const rapidjson::Value &val;

    void into(std::tuple<Ts...> &tpl) const {
        auto flattened        = flatten(tpl);
        using Tpl             = decltype(flattened);
        constexpr bool is_obj = cpx::detail::tuple_has_any_tagged_type_v<Tpl>;

        if (!is_obj && !val.IsArray())
            throw type_mismatch_error("array", cpx::json::rapid_json::detail::type(val));
        if (is_obj && !val.IsObject())
            throw type_mismatch_error("table", cpx::json::rapid_json::detail::type(val));

        size_t idx = 0;
        tuple_for_each(flattened, [&](auto &item, const size_t) {
            const cpx::TagInfo &t         = cpx::json::get_tag_info(item);
            auto               &v         = detail::get_underlying_value(item);
            using T                       = std::decay_t<decltype(v)>;
            constexpr bool deserializable = cpx::serde::is_deserializable_v<rapidjson::Value, T>;

            if (!deserializable || (is_obj && t.key == ""))
                return;

            const size_t            i = idx++;
            const rapidjson::Value  empty;
            const rapidjson::Value *ptr = &empty;
            if (is_obj) {
                auto obj = val.GetObject();
                if (auto it = obj.FindMember(rapidjson::StringRef(t.key.data(), t.key.size())); it != obj.end())
                    ptr = &it->value;
            } else {
                auto arr = val.GetArray();
                if (i < arr.Size())
                    ptr = &arr[i];
            }
            if (ptr == &empty && t.skipmissing)
                return;

            try {
                if (t.noserde)
                    if constexpr (std::is_same_v<T, std::string>)
                        Deserialize<rapidjson::Value, std::string>{*ptr}.into_raw(v);
                    else
                        throw error("field with tag `noserde` can only be deserialized into std::string");
                else {
                    if constexpr (is_deserializable_v<rapidjson::Value, T>)
                        Deserialize<rapidjson::Value, T>{*ptr}.into(v);
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
struct cpx::serde::Serialize<rapidjson::Value, T, std::enable_if_t<cpx::json::has_reflect_v<T>>> {
    rapidjson::Document &doc;

    rapidjson::Value from(const T &v) const {
        return Serialize<rapidjson::Value, cpx::json::const_reflect_t<T>>{doc}.from(cpx::json::reflect_of(v));
    }
};

template <typename T>
struct cpx::serde::Deserialize<rapidjson::Value, T, std::enable_if_t<cpx::json::has_reflect_v<T>>> {
    const rapidjson::Value &val;

    void into(T &v) const {
        decltype(auto) r = cpx::json::reflect_of(v);
        cpx::serde::Deserialize<rapidjson::Value, cpx::json::reflect_t<T>>{val}.into(r);
    }
};

// --- SAX ----

// bool and number
template <typename OS, typename T>
struct cpx::serde::Serialize<rapidjson::Writer<OS>, T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>> {
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
        if (res) {
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
            v = T(i);
            return true;
        } else {
            return false;
        }
    }

    bool Uint64(uint64_t u) override {
        if constexpr (std::is_same_v<T, uint64_t>) {
            v = T(u);
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
template <typename OS, typename CT, typename A>
struct cpx::serde::Serialize<rapidjson::Writer<OS>, std::basic_string<char, CT, A>> {
    rapidjson::Writer<OS> &writer;

    void from(const std::basic_string<char, CT, A> &v) const {
        if (!writer.WriteString(v.c_str(), v.size()))
            throw error("error"); // TODO
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
struct cpx::serde::Serialize<
    rapidjson::Writer<OS>,
    std::optional<T>,
    std::enable_if_t<cpx::serde::is_serializable_v<rapidjson::Writer<OS>, T>>> {
    rapidjson::Writer<OS> &writer;

    void from(const std::optional<T> &v) const {
        if (v.has_value())
            Serialize<rapidjson::Writer<OS>, T>{writer}.from(*v);
        else
            writer.Null();
    }
};

// TODO
template <typename T>
struct cpx::serde::Deserialize<
    rapidjson::Reader,
    std::optional<T>,
    std::enable_if_t<cpx::serde::is_deserializable_v<rapidjson::Reader, T> && std::is_default_constructible_v<T>>>
    : public cpx::json::rapid_json::detail::HandlerFor<T> {
    Deserialize(rapidjson::Reader &reader, std::optional<T> &v)
        : cpx::json::rapid_json::detail::HandlerFor<T>(reader) {
        v = T{};
    }

    bool Null() override {
        this->v = std::nullopt;
        return true;
    }
};


namespace cpx::josn::rapid_json {
    template <typename T>
    void parse(const std::string &str, T &val) {
        cpx::json::rapid_json::Parse<std::string>{str}.into(val);
    }

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(const std::string &str) {
        T val = {};
        cpx::json::rapid_json::Parse<std::string>{str}.into(val);
        return val;
    }

    template <typename T>
    [[nodiscard]]
    std::string dump(const T &val) {
        return cpx::json::rapid_json::Dump<std::string>{}.from(val);
    }
} // namespace cpx::josn::rapid_json
#endif
