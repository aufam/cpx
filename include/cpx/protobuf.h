// TODO implement SerializedSize

#ifndef CPX_PROTO_PROTOBUF_H
#define CPX_PROTO_PROTOBUF_H

#include <cpx/protobuf_reflect.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <cpx/defer.h>
#include <array>
#include <tuple>
#include <string>
#include <vector>
#include <unordered_map>

#ifndef GOOGLE_PROTOBUF_IO_CODED_STREAM_H__
#    include <google/protobuf/io/coded_stream.h>
#endif

#ifndef GOOGLE_PROTOBUF_IO_ZERO_COPY_STREAM_IMPL_LITE_H__
#    include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#endif

#ifndef GOOGLE_PROTOBUF_IO_ZERO_COPY_STREAM_IMPL_H__
#    include <google/protobuf/io/zero_copy_stream_impl.h>
#endif

#ifndef GOOGLE_PROTOBUF_WIRE_FORMAT_LITE_H__
#    include <google/protobuf/wire_format_lite.h>
#endif

namespace cpx::protobuf {
    CPX_EXPORT template <typename T>
    constexpr decltype(auto) get_tag_info(const T &field) {
        if constexpr (cpx::detail::is_tag_info_for_v<T>)
            return field.ti;
        else
            return cpx::get_tag_info(field, "protobuf");
    }
} // namespace cpx::protobuf

#define SERIALIZE(...)      cpx::serde::Serialize<google::protobuf::io::CodedOutputStream, __VA_ARGS__>
#define DESERIALIZE(...)    cpx::serde::Deserialize<google::protobuf::io::CodedInputStream, __VA_ARGS__>
#define SERIALIZABLE(...)   cpx::serde::is_serializable_v<google::protobuf::io::CodedOutputStream, __VA_ARGS__>
#define DESERIALIZABLE(...) cpx::serde::is_deserializable_v<google::protobuf::io::CodedInputStream, __VA_ARGS__>
#define DUMP(...)           cpx::serde::Dump<google::protobuf::io::CodedOutputStream, __VA_ARGS__>
#define PARSE(...)          cpx::serde::Parse<google::protobuf::io::CodedInputStream, __VA_ARGS__>

namespace cpx::protobuf {
    CPX_EXPORT template <typename From>
    using Serialize = SERIALIZE(From);

    CPX_EXPORT template <typename To>
    using Deserialize = DESERIALIZE(To);

    CPX_EXPORT template <typename From>
    constexpr bool is_serializable_v = SERIALIZABLE(From);

    CPX_EXPORT template <typename To>
    constexpr bool is_deserializable_v = DESERIALIZABLE(To);

    CPX_EXPORT template <typename To>
    using Dump = DUMP(To);

    CPX_EXPORT template <typename From>
    using Parse = PARSE(From);

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::string dump(const T &val);

    CPX_EXPORT template <typename T>
    void dump(std::ostream &os, const T &val);

    CPX_EXPORT template <typename T>
    void parse(std::string_view str, T &val);

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(std::string_view str);

    CPX_EXPORT template <typename T>
    void parse(std::istream &is, T &val);

    CPX_EXPORT template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_default_constructible_v<T>, T> parse(std::istream &is);
} // namespace cpx::protobuf

namespace cpx::protobuf::detail {
    // numeric
    template <typename T>
    struct is_numeric : std::bool_constant<std::is_arithmetic_v<T> || std::is_enum_v<T>> {};

    // bytes
    template <typename T>
    struct is_bytes : std::false_type {};

    template <size_t N>
    struct is_bytes<std::array<uint8_t, N>> : std::true_type {};

    template <typename A>
    struct is_bytes<std::vector<uint8_t, A>> : std::true_type {};

    template <typename CT, typename A>
    struct is_bytes<std::basic_string<char, CT, A>> : std::true_type {};

    template <typename CT>
    struct is_bytes<std::basic_string_view<char, CT>> : std::true_type {};

    // repeated numeric
    template <typename T>
    struct is_repeated_numeric : std::false_type {};

    template <typename T, size_t N>
    struct is_repeated_numeric<std::array<T, N>> : std::bool_constant<is_numeric<T>::value && !std::is_same_v<T, uint8_t>> {};

    template <typename T, typename A>
    struct is_repeated_numeric<std::vector<T, A>> : std::bool_constant<is_numeric<T>::value && !std::is_same_v<T, uint8_t>> {};

    // repeated message
    template <typename T>
    struct is_repeated : std::false_type {};

    template <typename T, size_t N>
    struct is_repeated<std::array<T, N>> : std::bool_constant<!is_numeric<T>::value && !std::is_same_v<T, uint8_t>> {};

    template <typename T, typename A>
    struct is_repeated<std::vector<T, A>> : std::bool_constant<!is_numeric<T>::value && !std::is_same_v<T, uint8_t>> {};

    // repeated serializable
    template <typename T>
    struct is_repeated_serializable : std::false_type {};

    template <typename T, size_t N>
    struct is_repeated_serializable<std::array<T, N>>
        : std::bool_constant<
              SERIALIZABLE(T) && !is_repeated_numeric<std::array<T, N>>::value && !is_bytes<std::array<T, N>>::value
          > {};

    template <typename T, typename A>
    struct is_repeated_serializable<std::vector<T, A>>
        : std::bool_constant<
              SERIALIZABLE(T) && !is_repeated_numeric<std::vector<T, A>>::value && !is_bytes<std::vector<T, A>>::value
          > {};

    // repeated deserializable
    template <typename T>
    struct is_repeated_deserializable : std::false_type {};

    template <typename T, size_t N>
    struct is_repeated_deserializable<std::array<T, N>>
        : std::bool_constant<
              DESERIALIZABLE(T) && !is_repeated_numeric<std::array<T, N>>::value && !is_bytes<std::array<T, N>>::value
          > {};

    template <typename T, typename A>
    struct is_repeated_deserializable<std::vector<T, A>>
        : std::bool_constant<
              DESERIALIZABLE(T) && !is_repeated_numeric<std::vector<T, A>>::value && !is_bytes<std::vector<T, A>>::value
          > {};
} // namespace cpx::protobuf::detail

#define SERIALIZER_FIELDS                                                                                                        \
    google::protobuf::io::CodedOutputStream &doc;                                                                                \
    const cpx::TagInfo                      &ti = {};

#define DESERIALIZER_FIELDS                                                                                                      \
    google::protobuf::io::CodedInputStream              &doc;                                                                    \
    const cpx::TagInfo                                  &ti        = {};                                                         \
    google::protobuf::internal::WireFormatLite::WireType wire_type = {};                                                         \
    size_t                                               len       = 0;

#define DESERIALIZER_BODY                                                                                                        \
    const uint32_t field_number = tag >> 3;                                                                                      \
    const auto     wire_type    = static_cast<google::protobuf::internal::WireFormatLite::WireType>(tag & 0x07);                 \
    uint64_t       len          = 0;                                                                                             \
    int            limit        = -1;                                                                                            \
    if (wire_type == google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {                                    \
        if (!doc.ReadVarint64(&len))                                                                                             \
            throw error("failed reading length");                                                                                \
        if (len > INT_MAX)                                                                                                       \
            throw error("message too large");                                                                                    \
        limit = doc.PushLimit(static_cast<int>(len));                                                                            \
    }                                                                                                                            \
    cpx::defer _ = [&]() {                                                                                                       \
        if (limit >= 0)                                                                                                          \
            doc.PopLimit(limit);                                                                                                 \
    };

// numeric
template <typename T>
struct SERIALIZE(T, std::enable_if_t<cpx::protobuf::detail::is_numeric<T>::value>) {
    SERIALIZER_FIELDS

    bool from(T v) const {
        if ((ti.omitempty || !ti.oneof.empty()) && v == T())
            return false;

        if (ti.field_number > 0) {
            auto tag = google::protobuf::internal::WireFormatLite::MakeTag(
                ti.field_number,
                std::is_same_v<T, double>       ? google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64
                : std::is_same_v<T, float>      ? google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED32
                : !ti.fixed                     ? google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT
                : sizeof(T) == sizeof(uint64_t) ? google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64
                                                : google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED32
            );
            doc.WriteTag(tag);
        }

        if constexpr (std::is_same_v<T, double>) {
            doc.WriteLittleEndian64(google::protobuf::internal::WireFormatLite::EncodeDouble(v));
        } else if constexpr (std::is_same_v<T, float>) {
            doc.WriteLittleEndian32(google::protobuf::internal::WireFormatLite::EncodeFloat(v));
        } else {
            if (ti.fixed) {
                if constexpr (sizeof(T) == 8)
                    doc.WriteLittleEndian64(static_cast<uint64_t>(v));
                else
                    doc.WriteLittleEndian32(static_cast<uint32_t>(v));
            } else if (ti.zigzag) {
                if constexpr (sizeof(T) == 8)
                    doc.WriteVarint64(google::protobuf::internal::WireFormatLite::ZigZagEncode64(static_cast<int64_t>(v)));
                else
                    doc.WriteVarint32(google::protobuf::internal::WireFormatLite::ZigZagEncode32(static_cast<int32_t>(v)));
            } else {
                if constexpr (sizeof(T) == 8)
                    doc.WriteVarint64(static_cast<uint64_t>(v));
                else
                    doc.WriteVarint32(static_cast<uint32_t>(v));
            }
        }

        return true;
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<cpx::protobuf::detail::is_numeric<T>::value>) {
    DESERIALIZER_FIELDS

    void into(T &v) const {
        wire_type == google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED32 ? read32(v) : read64(v);
    }

    void read32(T &v) const {
        uint32_t bits;
        if (!doc.ReadLittleEndian32(&bits))
            throw serde::error("failed to decode int32");
        if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
            v = (T)google::protobuf::internal::WireFormatLite::DecodeFloat(bits);
        } else {
            if (ti.zigzag)
                v = static_cast<T>(google::protobuf::internal::WireFormatLite::ZigZagDecode32(bits));
            else
                v = static_cast<T>(bits);
        }
    }

    void read64(T &v) const {
        uint64_t bits;
        if (!(wire_type == google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64 ? doc.ReadLittleEndian64(&bits)
                                                                                        : doc.ReadVarint64(&bits)))
            throw serde::error("failed to decode int64");

        if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
            v = (T)google::protobuf::internal::WireFormatLite::DecodeDouble(bits);
        } else {
            if (ti.zigzag)
                v = static_cast<T>(google::protobuf::internal::WireFormatLite::ZigZagDecode64(bits));
            else
                v = static_cast<T>(bits);
        }
    }
};

// bytes and string
template <typename T>
struct SERIALIZE(T, std::enable_if_t<cpx::protobuf::detail::is_bytes<T>::value>) {
    SERIALIZER_FIELDS

    bool from(const T &v) const {
        if ((ti.omitempty || !ti.oneof.empty()) && v.empty())
            return false;

        if (ti.field_number > 0) {
            auto tag = google::protobuf::internal::WireFormatLite::MakeTag(
                ti.field_number, google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED
            );
            doc.WriteTag(tag);
            doc.WriteVarint64(v.size());
        }
        doc.WriteRaw(v.data(), (int)v.size());
        return true;
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<cpx::protobuf::detail::is_bytes<T>::value>) {
    DESERIALIZER_FIELDS

    void into(T &v) const {
        if (wire_type != google::protobuf::internal::WireFormatLite::WireType::WIRETYPE_LENGTH_DELIMITED)
            throw error("expect length delimited got wire_type=" + std::to_string(wire_type));

        bool ok = false;
        if constexpr (std::is_same_v<T, std::string>) {
            ok = doc.ReadString(&v, (int)len);
        } else if constexpr (std::is_same_v<T, std::string_view>) {
            const void *ptr;
            int         available;
            if (doc.GetDirectBufferPointer(&ptr, &available)) {
                if ((int)len != available)
                    throw size_mismatch_error(len, size_t(available));
                v           = std::string_view(static_cast<const char *>(ptr), available);
                std::ignore = doc.Skip((int)len);
                ok          = true;
            } else {
                throw error("cannot get bytes view for this reader");
            }
        } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
            v.resize(len);
            ok = doc.ReadRaw(v.data(), (int)len);
        } else {
            ok = doc.ReadRaw(v.data(), (int)len);
        }

        if (!ok)
            throw error("failed to deserialize bytes");
    }
};

// repeated numeric
template <typename T>
struct SERIALIZE(T, std::enable_if_t<cpx::protobuf::detail::is_repeated_numeric<T>::value>) {
    SERIALIZER_FIELDS

    bool from(const T &arr) const {
        bool ret = false;
        if (ti.packed) {
            std::string buffer = create_packed_buffer(arr);
            ret                = SERIALIZE(std::string){doc, ti}.from(buffer);
        } else {
            auto ti        = this->ti;
            ti.omitempty   = false;
            ti.oneof       = "";
            const auto ser = SERIALIZE(typename T::value_type){doc, ti};
            for (auto &v : arr)
                ret |= ser.from(v);
        }
        return ret;
    }

    std::string create_packed_buffer(const T &arr) const {
        std::string                              buffer;
        google::protobuf::io::StringOutputStream os(&buffer);
        google::protobuf::io::CodedOutputStream  doc(&os);

        auto ti         = this->ti;
        ti.field_number = 0;
        ti.omitempty    = false;
        ti.oneof        = "";

        SERIALIZE(typename T::value_type) ser = {doc, ti};
        for (auto &v : arr)
            ser.from(v);

        return buffer;
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<cpx::protobuf::detail::is_repeated_numeric<T>::value>) {
    DESERIALIZER_FIELDS
    using value_type = typename T::value_type;

    void into(T &v) const {
        // TODO std::array ?
        if constexpr (cpx::detail::is_std_array<T>::value) {
            read_packed(v);
        } else {
            if (ti.packed) {
                read_packed(v);
            } else {
                value_type val;
                DESERIALIZE(value_type){doc, ti, wire_type, len}.into(val);
                v.push_back(val);
            }
        }
    }

    void read_packed(T &v) const {
        const bool fixed     = std::is_same_v<value_type, double> || std::is_same_v<value_type, float> || ti.fixed;
        const auto wire_type = //
            !fixed                                ? google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT
            : sizeof(typename T::value_type) == 8 ? google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64
                                                  : google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED32;

        size_t i = 0;
        for (T &arr = v; doc.BytesUntilLimit();)
            if constexpr (cpx::detail::is_std_array<T>::value) {
                if (i == v.size())
                    throw size_mismatch_error(v.size(), i);
                DESERIALIZE(value_type){doc, ti, wire_type}.into(arr.at(i++));
            } else {
                value_type val;
                DESERIALIZE(value_type){doc, ti, wire_type}.into(val);
                arr.push_back(val);
            }
    }
};

// optional
template <typename T>
struct SERIALIZE(std::optional<T>, std::enable_if_t<SERIALIZABLE(T)>) {
    SERIALIZER_FIELDS

    bool from(const std::optional<T> &v) const {
        if (v.has_value())
            return SERIALIZE(T){doc, ti}.from(*v);
        return false;
    }
};

template <typename T>
struct DESERIALIZE(std::optional<T>, std::enable_if_t<DESERIALIZABLE(T) && std::is_default_constructible_v<T>>) {
    DESERIALIZER_FIELDS

    void into(std::optional<T> &v) const {
        v.emplace();
        DESERIALIZE(T){doc, ti, wire_type, len}.into(*v);
    }
};

// repeated
template <typename T>
struct SERIALIZE(T, std::enable_if_t<cpx::protobuf::detail::is_repeated_serializable<T>::value>) {
    SERIALIZER_FIELDS

    bool from(const T &arr) const {
        bool ret = false;
        for (const auto &v : arr) {
            using VT     = std::decay_t<decltype(v)>;
            auto ti      = this->ti;
            ti.omitempty = false;
            ti.oneof     = "";
            ret |= SERIALIZE(VT){doc, ti}.from(v);
        }
        return ret;
    }
};

template <typename T>
struct DESERIALIZE(
    T, std::enable_if_t<cpx::protobuf::detail::is_repeated_deserializable<T>::value && cpx::detail::is_std_array<T>::value>
) {
    DESERIALIZER_FIELDS

    void into(T &) const {
        throw error("TODO: not implemented");
    }
};

template <typename T>
struct DESERIALIZE(
    T, std::enable_if_t<cpx::protobuf::detail::is_repeated_deserializable<T>::value && !cpx::detail::is_std_array<T>::value>
) {
    DESERIALIZER_FIELDS
    using value_type = typename T::value_type;

    void into(T &arr) const {
        value_type v = {};
        DESERIALIZE(value_type){doc, ti, wire_type, len}.into(v);
        arr.push_back(std::move(v));
    }
};

// tuple
template <typename... Ts>
struct SERIALIZE(std::tuple<Ts...>) {
    SERIALIZER_FIELDS

    bool from(const std::tuple<Ts...> &tpl) const {
        std::string buffer;
        bool        ret = false;
        {
            google::protobuf::io::StringOutputStream os(&buffer);
            google::protobuf::io::CodedOutputStream  sdoc(&os);

            auto  &doc         = ti.field_number > 0 ? sdoc : this->doc;
            auto   flattened   = flatten(tpl);
            auto   oneofs      = std::array<std::string_view, std::tuple_size_v<decltype(flattened)>>();
            size_t oneof_count = 0;
            tuple_for_each(flattened, [&](const auto &item, size_t) {
                const cpx::TagInfo &t = cpx::protobuf::get_tag_info(item);
                const auto         &v = detail::get_underlying_value(item);
                using T               = std::decay_t<decltype(v)>;

                bool written = false;
                if constexpr (SERIALIZABLE(T))
                    if (ti.field_number > 0)
                        written = SERIALIZE(T){doc, ti}.from(v);

                if (written && !t.oneof.empty()) {
                    for (size_t j = 0; j < oneof_count; ++j) {
                        if (oneofs[j] == t.oneof)
                            throw duplicate_oneof_error(t.oneof);
                    }
                    oneofs[oneof_count++] = t.oneof;
                }

                ret |= written;
            });
        }
        if (ti.field_number > 0)
            ret = SERIALIZE(std::string){doc, ti}.from(buffer);

        return ret;
    }
};

template <typename... Ts>
struct DESERIALIZE(std::tuple<Ts...>) {
    DESERIALIZER_FIELDS

    void into(std::tuple<Ts...> &tpl) const {
        auto             flattened = cpx::flatten(tpl);
        constexpr size_t size      = std::tuple_size_v<decltype(flattened)>;

        std::array<cpx::TagInfo, size> tis = {};
        tuple_for_each(flattened, [&](auto &v, size_t i) { tis[i] = cpx::protobuf::get_tag_info(v); });

        auto   oneofs      = std::array<std::string_view, size>{};
        size_t oneof_count = 0;
        while (const uint32_t tag = doc.ReadTag()) {
            DESERIALIZER_BODY

            bool done = false;
            tuple_for_each(flattened, [&](auto &item, size_t i) {
                const cpx::TagInfo &t = tis[i];
                if (done || (int)field_number != t.field_number)
                    return;

                auto &v = detail::get_underlying_value(item);
                using T = std::decay_t<decltype(v)>;

                if constexpr (DESERIALIZABLE(T)) {
                    try {
                        DESERIALIZE(T){doc, t, wire_type, len}.into(v);
                    } catch (serde::error &e) {
                        e.add_context(std::to_string(field_number));
                        throw;
                    }
                    if (!t.oneof.empty()) {
                        for (size_t j = 0; j < oneof_count; ++j) {
                            if (oneofs[j] == t.oneof)
                                throw duplicate_oneof_error(t.oneof);
                        }
                        oneofs[oneof_count++] = t.oneof;
                    }
                    done = true;
                }
            });

            if (!done) {
                if (wire_type == google::protobuf::internal::WireFormatLite::WireType::WIRETYPE_LENGTH_DELIMITED)
                    std::ignore = doc.Skip((int)len);
                else
                    std::ignore = google::protobuf::internal::WireFormatLite::SkipField(&doc, tag);
            }
        }
    }
};

// map
template <typename K, typename T, typename H, typename P, typename A>
struct SERIALIZE(std::unordered_map<K, T, H, P, A>, std::enable_if_t<SERIALIZABLE(K) && SERIALIZABLE(T)>) {
    SERIALIZER_FIELDS

    bool from(const std::unordered_map<K, T, H, P, A> &map) const {
        cpx::TagInfo tag_key = ti;
        cpx::TagInfo tag_val = ti;
        tag_key.omitempty    = false;
        tag_key.oneof        = "";
        tag_val.oneof        = "";
        tag_key.field_number = 1;
        tag_val.field_number = 2;

        bool ret = false;
        for (const auto &[k, v] : map) {
            std::string entry_buffer;
            {
                google::protobuf::io::StringOutputStream os(&entry_buffer);
                google::protobuf::io::CodedOutputStream  entry_doc(&os);

                SERIALIZE(K){entry_doc, tag_key}.from(k);
                SERIALIZE(T){entry_doc, tag_val}.from(v);
            }
            ret |= SERIALIZE(std::string){doc, ti}.from(entry_buffer);
        }
        return ret;
    }
};

template <typename K, typename T, typename H, typename P, typename A>
struct DESERIALIZE(
    std::unordered_map<K, T, H, P, A>,
    std::enable_if_t<
        DESERIALIZABLE(K) && DESERIALIZABLE(T) && std::is_default_constructible_v<K> && std::is_default_constructible_v<T>
    >
) {
    DESERIALIZER_FIELDS

    void into(std::unordered_map<K, T, H, P, A> &map) const {
        if (wire_type != google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED)
            throw error("deserializing map requires length delimited got wire_type=" + std::to_string(wire_type));

        K k = {};
        T v = {};
        while (const uint32_t tag = doc.ReadTag()) {
            DESERIALIZER_BODY

            try {
                if (field_number == 1) {
                    DESERIALIZE(K){doc, ti, wire_type, len}.into(k);
                    std::ignore = map[k];
                } else if (field_number == 2) {
                    DESERIALIZE(T){doc, ti, wire_type, len}.into(v);
                    map[k] = std::move(v);
                } else {
                    throw error("expect field_number 1 or 2 got " + std::to_string(field_number));
                }
            } catch (error &e) {
                e.add_context(std::to_string(field_number));
                throw;
            }
        }
    }
};

// tm
template <>
struct SERIALIZE(std::tm) {
    SERIALIZER_FIELDS

    bool from(const std::tm &v) const {
        std::tm tm  = v;
        time_t  sec = timegm(&tm);

        cpx::TagInfo tag_sec = ti;
        tag_sec.field_number = 1;
        tag_sec.omitempty    = true;

        std::string buffer;
        bool        ret = false;
        {
            google::protobuf::io::StringOutputStream os(&buffer);
            google::protobuf::io::CodedOutputStream  sdoc(&os);

            auto &doc = ti.field_number > 0 ? sdoc : this->doc;

            ret |= SERIALIZE(time_t){doc, tag_sec}.from(sec);
        };
        if (ti.field_number > 0)
            ret = SERIALIZE(std::string){doc, ti}.from(buffer);

        return ret;
    }
};

template <>
struct DESERIALIZE(std::tm) {
    DESERIALIZER_FIELDS

    void into(std::tm &tm) const {
        while (const uint32_t tag = doc.ReadTag()) {
            DESERIALIZER_BODY

            time_t sec;
            try {
                if (field_number == 1) {
                    DESERIALIZE(time_t){doc, ti, wire_type, len}.into(sec);
                    tm = *std::gmtime(&sec);
                } else {
                    throw error("expect field_number 1 or 2 got " + std::to_string(field_number));
                }
            } catch (serde::error &e) {
                e.add_context(std::to_string(field_number));
                throw;
            }
        }
    }
};

// timespec
template <>
struct SERIALIZE(std::timespec) {
    SERIALIZER_FIELDS

    bool from(const std::timespec &v) const {
        cpx::TagInfo tag_sec  = ti;
        cpx::TagInfo tag_nsec = ti;
        tag_sec.field_number  = 1;
        tag_nsec.field_number = 2;
        tag_sec.omitempty = tag_nsec.omitempty = true;

        std::string buffer;
        bool        ret = false;
        {
            google::protobuf::io::StringOutputStream os(&buffer);
            google::protobuf::io::CodedOutputStream  sdoc(&os);

            auto &doc = ti.field_number > 0 ? sdoc : this->doc;

            ret |= SERIALIZE(decltype(std::timespec::tv_sec)){doc, tag_sec}.from(v.tv_sec);
            ret |= SERIALIZE(decltype(std::timespec::tv_sec)){doc, tag_nsec}.from(v.tv_nsec);
        };
        if (ti.field_number > 0)
            ret = SERIALIZE(std::string){doc, ti}.from(buffer);

        return ret;
    }
};

template <>
struct DESERIALIZE(std::timespec) {
    DESERIALIZER_FIELDS

    void into(std::timespec &v) const {
        while (const uint32_t tag = doc.ReadTag()) {
            DESERIALIZER_BODY

            try {
                if (field_number == 1) {
                    DESERIALIZE(decltype(std::timespec::tv_sec)){doc, ti, wire_type, len}.into(v.tv_sec);
                } else if (field_number == 2) {
                    DESERIALIZE(decltype(std::timespec::tv_nsec)){doc, ti, wire_type, len}.into(v.tv_nsec);
                } else {
                    throw error("expect field_number 1 or 2 got " + std::to_string(field_number));
                }
            } catch (error &e) {
                e.add_context(std::to_string(field_number));
                throw;
            }
        }
    }
};

// reflect
template <typename T>
struct SERIALIZE(T, std::enable_if_t<cpx::protobuf::has_reflect_v<T>>) {
    SERIALIZER_FIELDS

    bool from(const T &v) const {
        using traits = cpx::protobuf::reflect_traits<T>;
        if constexpr (traits::has_to_bytes) {
            std::string str;
            traits::to_bytes(v, str);
            return SERIALIZE(std::string){doc, ti}.from(str);
        } else if constexpr (traits::has_to_str) {
            std::string str;
            traits::to_str(v, str);
            return SERIALIZE(std::string){doc, ti}.from(str);
        } else {
            return SERIALIZE(typename traits::const_type){doc, ti}.from(traits::of(v));
        }
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<cpx::protobuf::has_reflect_v<T>>) {
    DESERIALIZER_FIELDS

    void into(T &v) const {
        using traits = cpx::protobuf::reflect_traits<T>;
        if constexpr (traits::has_from_bytes) {
            std::string r;
            DESERIALIZE(std::string){doc, ti, wire_type, len}.into(r);
            traits::from_bytes(v, r);
        } else if constexpr (traits::has_from_str) {
            std::string r;
            DESERIALIZE(std::string){doc, ti, wire_type, len}.into(r);
            traits::from_str(v, r);
        } else {
            decltype(auto) r = traits::of(v);
            DESERIALIZE(typename traits::type){doc, ti, wire_type, len}.into(r);
        }
    }
};

template <>
struct DUMP(std::string) {
    template <typename T>
    std::string from(const T &v) const {
        std::string                              buffer;
        google::protobuf::io::StringOutputStream os(&buffer);
        google::protobuf::io::CodedOutputStream  doc(&os);
        SERIALIZE(T){doc}.from(v);
        return buffer;
    }
};

template <size_t N>
struct DUMP(std::array<uint8_t, N>) {
    template <typename T>
    std::array<uint8_t, N> from(const T &v) const {
        std::array<uint8_t, N>                  buffer;
        google::protobuf::io::ArrayOutputStream os(buffer.data(), (int)buffer.size());
        google::protobuf::io::CodedOutputStream doc(&os);
        SERIALIZE(T){doc}.from(v);
        return buffer;
    }
};

template <>
struct DUMP(std::vector<uint8_t>) {
    template <typename T>
    std::vector<uint8_t> from(const T &v, size_t capacity) const {
        std::vector<uint8_t>                    buffer(capacity);
        google::protobuf::io::ArrayOutputStream os(buffer.data(), (int)buffer.size());
        google::protobuf::io::CodedOutputStream doc(&os);
        SERIALIZE(T){doc}.from(v);
        buffer.resize(size_t(doc.ByteCount()));
        return buffer;
    }
};

template <>
struct DUMP(std::ostream) {
    std::ostream &stream;

    template <typename T>
    std::ostream &from(const T &v) const {
        google::protobuf::io::OstreamOutputStream os(&stream);
        google::protobuf::io::CodedOutputStream   doc(&os);
        SERIALIZE(T){doc}.from(v);
        return stream;
    }

    template <typename T>
    std::ostream &operator<<(const T &v) const {
        return from(v);
    }
};

template <>
struct PARSE(std::string_view) {
    std::string_view buffer;

    template <typename T>
    void into(T &v) const {
        google::protobuf::io::ArrayInputStream ais(buffer.data(), (int)buffer.size());
        google::protobuf::io::CodedInputStream doc(&ais);

        DESERIALIZE(T){doc}.into(v);
        if (!doc.ConsumedEntireMessage())
            throw error("message not fully consumed");
    }
};

template <>
struct PARSE(std::vector<uint8_t>) {
    const std::vector<uint8_t> &buffer;

    template <typename T>
    void into(T &v) const {
        google::protobuf::io::ArrayInputStream ais(buffer.data(), (int)buffer.size());
        google::protobuf::io::CodedInputStream doc(&ais);

        DESERIALIZE(T){doc}.into(v);
        if (!doc.ConsumedEntireMessage())
            throw error("message not fully consumed");
    }
};

template <>
struct PARSE(std::istream) {
    std::istream &stream;

    template <typename T>
    std::istream &into(T &v) const {
        google::protobuf::io::IstreamInputStream iis(&stream);
        google::protobuf::io::CodedInputStream   doc(&iis);

        DESERIALIZE(T){doc}.into(v);
        if (!doc.ConsumedEntireMessage())
            throw error("message not fully consumed");

        return stream;
    }

    template <typename T>
    std::istream &operator>>(T &v) const {
        return into(v);
    }
};

template <typename T>
[[nodiscard]]
std::string cpx::protobuf::dump(const T &val) {
    return Dump<std::string>{}.from(val);
}

template <typename T>
void cpx::protobuf::dump(std::ostream &os, const T &val) {
    Dump<std::ostream>{os}.from(val);
}

template <typename T>
void cpx::protobuf::parse(std::string_view str, T &val) {
    Parse<std::string_view>{str}.into(val);
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T> cpx::protobuf::parse(std::string_view str) {
    T val = {};
    Parse<std::string_view>{str}.into(val);
    return val;
}

template <typename T>
void cpx::protobuf::parse(std::istream &is, T &val) {
    Parse<std::istream>{is}.into(val);
}

template <typename T>
[[nodiscard]]
std::enable_if_t<std::is_default_constructible_v<T>, T> cpx::protobuf::parse(std::istream &is) {
    T val = {};
    Parse<std::istream>{is}.into(val);
    return val;
}

namespace cpx::protobuf {
    CPX_EXPORT inline constexpr class IO {
        friend Dump<std::ostream> operator<<(std::ostream &os, const IO &) {
            return {os};
        }

        friend Parse<std::istream> operator>>(std::istream &is, const IO &) {
            return {is};
        }
    } io;
} // namespace cpx::protobuf

#undef SERIALIZE
#undef DESERIALIZE
#undef SERIALIZABLE
#undef DESERIALIZABLE
#undef DUMP
#undef PARSE
#undef SERIALIZER_FIELDS
#undef DESERIALIZER_FIELDS
#undef DESERIALIZER_BODY
#endif
