#ifndef CPX_PROTO_PROTOBUF_H
#define CPX_PROTO_PROTOBUF_H

#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/reflect.h>
#include <cpx/extend.h>
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

namespace cpx::proto::protobuf {
    template <typename T>
    struct Reflect : std::false_type {
        using const_type = type;
    };

    template <typename T>
    struct has_reflect
        : std::bool_constant<(Reflect<T>::value || cpx::has_reflect_v<T>) && !cpx::is_time_v<T> && !std::is_enum_v<T>> {};

    template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    template <typename T>
    using reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::type, cpx::reflect_t<T>>;

    template <typename T>
    using const_reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::const_type, cpx::const_reflect_t<T>>;

    template <typename T>
    constexpr decltype(auto) reflect_of(T &&v) {
        if constexpr (Reflect<std::decay_t<T>>::value)
            return Reflect<std::decay_t<T>>::of(std::forward<T>(v));
        else
            return cpx::reflect_of(std::forward<T>(v));
    }


    template <typename From>
    using Serialize = cpx::serde::Serialize<google::protobuf::io::CodedOutputStream, From>;

    template <typename To>
    using Deserialize = cpx::serde::Deserialize<google::protobuf::io::CodedInputStream, To>;

    template <typename To>
    using Dump = cpx::serde::Dump<google::protobuf::io::CodedOutputStream, To>;

    template <typename From>
    using Parse = cpx::serde::Parse<google::protobuf::io::CodedInputStream, From>;

    template <typename T>
    [[nodiscard]]
    std::string dump(const T &val);

    template <typename T>
    void dump(std::ostream &os, const T &val);

    template <typename T>
    void parse(const std::string &str, T &val);

    template <typename T>
    [[nodiscard]]
    T parse(const std::string &str);

    template <typename T>
    void parse(std::istream &is, T &val);

    template <typename T>
    [[nodiscard]]
    T parse(std::istream &is);

    template <typename T>
    constexpr decltype(auto) get_tag_info(const T &field) {
        if constexpr (cpx::detail::is_tag_info_for_v<T>)
            return field.ti;
        else
            return cpx::get_tag_info(field, "protobuf");
    }
} // namespace cpx::proto::protobuf

namespace cpx::proto::protobuf::detail {
    template <typename T>
    struct is_numeric : std::bool_constant<std::is_arithmetic_v<T> || std::is_enum_v<T>> {};


    template <typename T>
    struct is_bytes : std::false_type {};

    template <size_t N>
    struct is_bytes<std::array<uint8_t, N>> : std::true_type {};

    template <typename A>
    struct is_bytes<std::vector<uint8_t, A>> : std::true_type {};

    template <typename CT, typename A>
    struct is_bytes<std::basic_string<char, CT, A>> : std::true_type {};


    template <typename T>
    struct is_repeated_numeric : std::false_type {};

    template <typename T, size_t N>
    struct is_repeated_numeric<std::array<T, N>> : std::bool_constant<is_numeric<T>::value && !std::is_same_v<T, uint8_t>> {};

    template <typename T, typename A>
    struct is_repeated_numeric<std::vector<T, A>> : std::bool_constant<is_numeric<T>::value && !std::is_same_v<T, uint8_t>> {};


    template <typename T>
    struct is_repeated : std::false_type {};

    template <typename T, size_t N>
    struct is_repeated<std::array<T, N>> : std::bool_constant<!is_numeric<T>::value && !std::is_same_v<T, uint8_t>> {};

    template <typename T, typename A>
    struct is_repeated<std::vector<T, A>> : std::bool_constant<!is_numeric<T>::value && !std::is_same_v<T, uint8_t>> {};


    template <typename T>
    struct is_repeated_serializable : std::false_type {};

    template <typename T, size_t N>
    struct is_repeated_serializable<std::array<T, N>>
        : std::bool_constant<
              !is_repeated_numeric<std::array<T, N>>::value && !is_bytes<std::array<T, N>>::value &&
              serde::is_serializable_v<google::protobuf::io::CodedOutputStream, T>> {};

    template <typename T, typename A>
    struct is_repeated_serializable<std::vector<T, A>>
        : std::bool_constant<
              !is_repeated_numeric<std::vector<T, A>>::value && !is_bytes<std::vector<T, A>>::value &&
              serde::is_serializable_v<google::protobuf::io::CodedOutputStream, T>> {};


    template <typename T>
    struct is_repeated_deserializable : std::false_type {};

    template <typename T, size_t N>
    struct is_repeated_deserializable<std::array<T, N>>
        : std::bool_constant<
              !is_repeated_numeric<std::array<T, N>>::value && !is_bytes<std::array<T, N>>::value &&
              serde::is_deserializable_v<google::protobuf::io::CodedInputStream, T>> {};

    template <typename T, typename A>
    struct is_repeated_deserializable<std::vector<T, A>>
        : std::bool_constant<
              !is_repeated_numeric<std::vector<T, A>>::value && !is_bytes<std::vector<T, A>>::value &&
              serde::is_deserializable_v<google::protobuf::io::CodedInputStream, T>> {};


    template <typename T>
    struct is_message : cpx::is_tuple<cpx::proto::protobuf::reflect_t<T>> {};

    template <typename... Ts>
    struct is_message<std::tuple<Ts...>> : std::true_type {};


    class DeserializeDispatcher {
    public:
        google::protobuf::io::CodedInputStream              &doc;
        google::protobuf::internal::WireFormatLite::WireType wire_type = {};
        size_t                                               len       = 0;
        bool                                                 fixed     = false;
        bool                                                 zigzag    = false;
        bool                                                 packed    = true;
        bool                                                 is_root   = false;

        explicit DeserializeDispatcher(google::protobuf::io::CodedInputStream &doc) noexcept
            : doc(doc) {}

        virtual ~DeserializeDispatcher() = default;

        DeserializeDispatcher &inherit(DeserializeDispatcher &other) {
            this->is_root   = other.is_root;
            this->wire_type = other.wire_type;
            this->len       = other.len;
            this->fixed     = other.fixed;
            this->zigzag    = other.zigzag;
            this->packed    = other.packed;
            return *this;
        }

        DeserializeDispatcher &set_root(bool root = true) {
            this->is_root = root;
            return *this;
        }

        DeserializeDispatcher &set_wire_type(google::protobuf::internal::WireFormatLite::WireType wire_type) {
            this->wire_type = wire_type;
            return *this;
        }

        DeserializeDispatcher &set_fixed(bool fixed) {
            this->fixed = fixed;
            return *this;
        }

        DeserializeDispatcher &set_zigzag(bool zigzag) {
            this->zigzag = zigzag;
            return *this;
        }

        DeserializeDispatcher &set_packed(bool packed) {
            this->packed = packed;
            return *this;
        }

        DeserializeDispatcher &set_len(size_t len) {
            this->len = len;
            return *this;
        }

        DeserializeDispatcher &read_len() {
            if (wire_type == google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED)
                doc.ReadVarint64(&len);
            return *this;
        }

        virtual void read() = 0;
    };

    template <typename T>
    class DeserializeDispatcherFor : public DeserializeDispatcher {
    public:
        using DeserializeDispatcher::DeserializeDispatcher;

        DeserializeDispatcherFor(google::protobuf::io::CodedInputStream &doc, T &val) noexcept
            : proto::protobuf::detail::DeserializeDispatcher(doc)
            , val(&val) {}

        ~DeserializeDispatcherFor() override = default;

        T *val = nullptr;

        void into(T &val) {
            this->val = &val;
            read();
        }
    };

    template <typename T>
    std::string to_message(const T &v);

    template <typename T>
    std::string read_message(DeserializeDispatcherFor<T> &d);
} // namespace cpx::proto::protobuf::detail

namespace cpx::serde {
    template <>
    struct Dump<google::protobuf::io::CodedOutputStream, std::string> {
        template <typename T>
        std::string from(const T &v) const {
            std::string                              buffer;
            google::protobuf::io::StringOutputStream os(&buffer);
            google::protobuf::io::CodedOutputStream  doc(&os);
            Serialize<google::protobuf::io::CodedOutputStream, T>{doc}.from(v);
            return buffer;
        }
    };

    template <size_t N>
    struct Dump<google::protobuf::io::CodedOutputStream, std::array<uint8_t, N>> {
        template <typename T>
        std::array<uint8_t, N> from(const T &v) const {
            std::array<uint8_t, N>                  buffer;
            google::protobuf::io::ArrayOutputStream os(buffer.data(), (int)buffer.size());
            google::protobuf::io::CodedOutputStream doc(&os);
            Serialize<google::protobuf::io::CodedOutputStream, T>{doc}.from(v);
            return buffer;
        }
    };

    template <>
    struct Dump<google::protobuf::io::CodedOutputStream, std::vector<uint8_t>> {
        template <typename T>
        std::vector<uint8_t> from(const T &v, size_t capacity) const {
            std::vector<uint8_t>                    buffer(capacity);
            google::protobuf::io::ArrayOutputStream os(buffer.data(), (int)buffer.size());
            google::protobuf::io::CodedOutputStream doc(&os);
            Serialize<google::protobuf::io::CodedOutputStream, T>{doc}.from(v);
            buffer.resize(size_t(doc.ByteCount()));
            return buffer;
        }
    };

    template <>
    struct Dump<google::protobuf::io::CodedOutputStream, std::ostream> {
        std::ostream &stream;

        template <typename T>
        std::ostream &from(const T &v) const {
            google::protobuf::io::OstreamOutputStream os(&stream);
            google::protobuf::io::CodedOutputStream   doc(&os);
            Serialize<google::protobuf::io::CodedOutputStream, T>{doc}.from(v);
            return stream;
        }

        template <typename T>
        std::ostream &operator<<(const T &v) const {
            return from(v);
        }
    };

    template <>
    struct Parse<google::protobuf::io::CodedInputStream, std::string> {
        const std::string &buffer;

        template <typename T>
        void into(T &v) const {
            google::protobuf::io::ArrayInputStream ais(buffer.data(), (int)buffer.size());
            google::protobuf::io::CodedInputStream doc(&ais);

            Deserialize<google::protobuf::io::CodedInputStream, T> des(doc);
            des.set_root().set_len(buffer.size());
            des.into(v);

            if (!doc.ConsumedEntireMessage())
                throw serde::error("message not fully consumed");
        }
    };

    template <>
    struct Parse<google::protobuf::io::CodedInputStream, std::istream> {
        std::istream &stream;

        template <typename T>
        std::istream &into(T &v) const {
            google::protobuf::io::IstreamInputStream iis(&stream);
            google::protobuf::io::CodedInputStream   doc(&iis);

            Deserialize<google::protobuf::io::CodedInputStream, T> des(doc);
            des.set_root();
            des.into(v);

            if (!doc.ConsumedEntireMessage())
                throw serde::error("message not fully consumed");

            return stream;
        }

        template <typename T>
        std::istream &operator>>(T &v) const {
            return into(v);
        }
    };

    // numeric
    template <typename T>
    struct Serialize<
        google::protobuf::io::CodedOutputStream,
        T,
        std::enable_if_t<proto::protobuf::detail::is_numeric<T>::value>> {
        google::protobuf::io::CodedOutputStream &doc;
        mutable bool                             fixed    = false;
        mutable bool                             zigzag   = false;
        mutable bool                             omitzero = true;

        void from(T v, const cpx::TagInfo &ti) const {
            if (omitzero && v == T())
                return;

            auto tag = google::protobuf::internal::WireFormatLite::MakeTag(
                ti.field_number,
                std::is_same_v<T, double>       ? google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64
                : std::is_same_v<T, float>      ? google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED32
                : !ti.fixed                     ? google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT
                : sizeof(T) == sizeof(uint64_t) ? google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64
                                                : google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED32
            );
            this->doc.WriteTag(tag);
            fixed  = ti.fixed;
            zigzag = ti.zigzag;
            from(v);
        }

        void from(const T &v) const {
            if constexpr (std::is_same_v<T, double>) {
                this->doc.WriteLittleEndian64(google::protobuf::internal::WireFormatLite::EncodeDouble(v));
            } else if constexpr (std::is_same_v<T, float>) {
                this->doc.WriteLittleEndian32(google::protobuf::internal::WireFormatLite::EncodeFloat(v));
            } else {
                if (fixed) {
                    if constexpr (sizeof(T) == 8)
                        this->doc.WriteLittleEndian64(static_cast<uint64_t>(v));
                    else
                        this->doc.WriteLittleEndian32(static_cast<uint32_t>(v));
                } else if (zigzag) {
                    if constexpr (sizeof(T) == 8)
                        this->doc.WriteVarint64(
                            google::protobuf::internal::WireFormatLite::ZigZagEncode64(static_cast<int64_t>(v))
                        );
                    else
                        this->doc.WriteVarint32(
                            google::protobuf::internal::WireFormatLite::ZigZagEncode32(static_cast<int32_t>(v))
                        );
                } else {
                    if constexpr (sizeof(T) == 8)
                        this->doc.WriteVarint64(static_cast<uint64_t>(v));
                    else
                        this->doc.WriteVarint32(static_cast<uint32_t>(v));
                }
            }
        }
    };

    template <typename T>
    struct Deserialize<google::protobuf::io::CodedInputStream, T, std::enable_if_t<proto::protobuf::detail::is_numeric<T>::value>>
        : public proto::protobuf::detail::DeserializeDispatcherFor<T> {

        using proto::protobuf::detail::DeserializeDispatcherFor<T>::DeserializeDispatcherFor;
        ~Deserialize() override = default;

        void read32() {
            uint32_t bits;
            if (!this->doc.ReadLittleEndian32(&bits))
                throw serde::error("failed to decode int");
            if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
                *this->val = (T)google::protobuf::internal::WireFormatLite::DecodeFloat(bits);
            } else {
                if (this->zigzag)
                    *this->val = static_cast<T>(google::protobuf::internal::WireFormatLite::ZigZagDecode32(bits));
                else
                    *this->val = static_cast<T>(bits);
            }
        }

        void read64() {
            uint64_t bits;
            if (!(this->wire_type == google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64
                      ? this->doc.ReadLittleEndian64(&bits)
                      : this->doc.ReadVarint64(&bits)))
                throw serde::error("failed to decode int");

            if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
                *this->val = (T)google::protobuf::internal::WireFormatLite::DecodeDouble(bits);
            } else {
                if (this->zigzag)
                    *this->val = static_cast<T>(google::protobuf::internal::WireFormatLite::ZigZagDecode64(bits));
                else
                    *this->val = static_cast<T>(bits);
            }
        }

        void read() override {
            this->wire_type == google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED32 ? read32() : read64();
        }
    };

    // bytes and string
    template <typename T>
    struct Serialize<google::protobuf::io::CodedOutputStream, T, std::enable_if_t<proto::protobuf::detail::is_bytes<T>::value>> {
        google::protobuf::io::CodedOutputStream &doc;

        mutable bool omitempty = true;

        void from(const T &v, const cpx::TagInfo &ti) const {
            if (omitempty && v.empty())
                return;

            auto tag = google::protobuf::internal::WireFormatLite::MakeTag(
                ti.field_number, google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED
            );
            doc.WriteTag(tag);
            doc.WriteVarint64(v.size());
            from(v);
        }

        void from(const T &v) const {
            doc.WriteRaw(v.data(), (int)v.size());
        }
    };

    template <typename T>
    struct Deserialize<google::protobuf::io::CodedInputStream, T, std::enable_if_t<proto::protobuf::detail::is_bytes<T>::value>>
        : public proto::protobuf::detail::DeserializeDispatcherFor<T> {

        using proto::protobuf::detail::DeserializeDispatcherFor<T>::DeserializeDispatcherFor;
        ~Deserialize() override = default;

        void read() override {
            if (!this->is_root && this->wire_type != google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED)
                throw serde::error("failed to deserialize bytes");

            bool ok = false;
            if constexpr (std::is_same_v<T, std::string>) {
                ok = this->doc.ReadString(this->val, (int)this->len);
            } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                this->val->resize(this->len);
                ok = this->doc.ReadRaw(this->val->data(), (int)this->len);
            } else {
                ok = this->doc.ReadRaw(this->val->data(), (int)this->len);
            }

            if (!ok)
                throw serde::error("failed to deserialize bytes");
        }
    };

    // repeated numeric
    template <typename T>
    struct Serialize<
        google::protobuf::io::CodedOutputStream,
        T,
        std::enable_if_t<proto::protobuf::detail::is_repeated_numeric<T>::value>> {
        google::protobuf::io::CodedOutputStream &doc;

        void from(const T &arr, const cpx::TagInfo &ti) const {
            if (!ti.packed) {
                auto ser     = Serialize<google::protobuf::io::CodedOutputStream, typename T::value_type>{this->doc};
                ser.omitzero = false;
                for (auto &v : arr)
                    ser.from(v, ti);
            } else {
                std::string buffer = create_packed_buffer(arr, ti);
                Serialize<google::protobuf::io::CodedOutputStream, std::string>{this->doc}.from(buffer, ti);
            }
        }

        void from(const T &arr) const {
            std::string buffer = create_packed_buffer(arr);
            Serialize<google::protobuf::io::CodedOutputStream, std::string>{this->doc}.from(buffer);
        }

        std::string create_packed_buffer(const T &arr, const cpx::TagInfo &ti = {}) const {
            std::string                              buffer;
            google::protobuf::io::StringOutputStream os(&buffer);
            google::protobuf::io::CodedOutputStream  doc(&os);

            Serialize<google::protobuf::io::CodedOutputStream, typename T::value_type> ser = {doc};

            ser.fixed  = ti.fixed;
            ser.zigzag = ti.zigzag;
            for (auto &v : arr)
                ser.from(v);

            return buffer;
        }
    };

    template <typename T>
    struct Deserialize<
        google::protobuf::io::CodedInputStream,
        T,
        std::enable_if_t<proto::protobuf::detail::is_repeated_numeric<T>::value>>
        : public proto::protobuf::detail::DeserializeDispatcherFor<T> {

        using proto::protobuf::detail::DeserializeDispatcherFor<T>::DeserializeDispatcherFor;
        ~Deserialize() override = default;

        size_t i = 0;

        void read() override {
            if (this->packed) {
                read_packed();
                return;
            }

            Deserialize<google::protobuf::io::CodedInputStream, typename T::value_type> des(this->doc);
            des.set_wire_type(this->wire_type).set_fixed(this->fixed).set_zigzag(this->zigzag);

            T &arr = *this->val;
            if constexpr (cpx::detail::is_std_array<T>::value) {
                des.into(arr[i++]);
            } else {
                typename T::value_type val;
                des.into(val);
                arr.push_back(val);
            }
        }

        void read_packed() {
            std::string buffer;
            {
                Deserialize<google::protobuf::io::CodedInputStream, std::string> des(this->doc);
                des.inherit(*this);
                des.into(buffer);
            }

            google::protobuf::io::ArrayInputStream ais(buffer.data(), (int)buffer.size());
            google::protobuf::io::CodedInputStream doc(&ais);
            doc.PushLimit((int)buffer.size());

            Deserialize<google::protobuf::io::CodedInputStream, typename T::value_type> des(doc);

            const bool fixed =
                std::is_same_v<typename T::value_type, double> || std::is_same_v<typename T::value_type, float> || this->fixed;

            const auto wire_type = !fixed ? google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT
                                   : sizeof(typename T::value_type) == 8
                                       ? google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED64
                                       : google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED32;

            des.set_wire_type(wire_type).set_fixed(fixed).set_zigzag(this->zigzag);
            for (T &arr = *this->val; doc.BytesUntilLimit();) {
                if constexpr (cpx::detail::is_std_array<T>::value) {
                    des.into(arr.at(i++));
                } else {
                    typename T::value_type val;
                    des.into(val);
                    arr.push_back(val);
                }
            }
        }
    };

    // optional
    template <typename T>
    struct Serialize<
        google::protobuf::io::CodedOutputStream,
        std::optional<T>,
        std::enable_if_t<is_serializable_v<google::protobuf::io::CodedOutputStream, T>>> {
        google::protobuf::io::CodedOutputStream &doc;

        void from(const std::optional<T> &v, const cpx::TagInfo &ti) const {
            if (v.has_value())
                Serialize<google::protobuf::io::CodedOutputStream, T>{doc}.from(*v, ti);
        }

        void from(const std::optional<T> &v) const {
            if (v.has_value())
                Serialize<google::protobuf::io::CodedOutputStream, T>{doc}.from(*v);
        }
    };

    template <typename T>
    struct Deserialize<
        google::protobuf::io::CodedInputStream,
        std::optional<T>,
        std::enable_if_t<is_deserializable_v<google::protobuf::io::CodedInputStream, T>>>
        : public proto::protobuf::detail::DeserializeDispatcherFor<std::optional<T>> {

        using proto::protobuf::detail::DeserializeDispatcherFor<std::optional<T>>::DeserializeDispatcherFor;
        ~Deserialize() override = default;

        void read() override {
            if (this->is_root && this->len == 0) {
                *this->val = std::nullopt;
                return;
            }

            if (!this->val->has_value())
                *this->val = T{};

            Deserialize<google::protobuf::io::CodedInputStream, T> des(this->doc);
            des.inherit(*this);
            des.into(this->val->value());
        }
    };

    // repeated
    template <typename T>
    struct Serialize<
        google::protobuf::io::CodedOutputStream,
        T,
        std::enable_if_t<proto::protobuf::detail::is_repeated_serializable<T>::value>> {
        google::protobuf::io::CodedOutputStream &doc;

        void from(const T &arr, const cpx::TagInfo &ti) const {
            for (const auto &v : arr) {
                Serialize<google::protobuf::io::CodedOutputStream, std::decay_t<decltype(v)>> ser{doc};
                if constexpr (proto::protobuf::detail::is_bytes<std::decay_t<decltype(v)>>::value)
                    ser.omitempty = false;
                ser.from(v, ti);
            }
        }

        void from(const T &) const {
            throw std::runtime_error("should not be here");
        }
    };

    template <typename T>
    struct Deserialize<
        google::protobuf::io::CodedInputStream,
        T,
        std::enable_if_t<proto::protobuf::detail::is_repeated_deserializable<T>::value>>
        : public proto::protobuf::detail::DeserializeDispatcherFor<T> {

        using proto::protobuf::detail::DeserializeDispatcherFor<T>::DeserializeDispatcherFor;
        ~Deserialize() override = default;

        size_t i = 0;

        void read() override {
            Deserialize<google::protobuf::io::CodedInputStream, typename T::value_type> des(this->doc);
            des.inherit(*this);

            T &arr = *this->val;
            if constexpr (cpx::detail::is_std_array<T>::value) {
                des.into(arr.at(i++));
            } else {
                arr.emplace_back();
                des.into(arr.back());
            }
        }
    };

    // tuple
    template <typename... Ts>
    struct Serialize<google::protobuf::io::CodedOutputStream, std::tuple<Ts...>> {
        google::protobuf::io::CodedOutputStream &doc;

        void from(const std::tuple<Ts...> &tpl, const cpx::TagInfo &ti) {
            std::string buffer = cpx::proto::protobuf::detail::to_message(tpl);
            Serialize<google::protobuf::io::CodedOutputStream, std::string>{this->doc}.from(buffer, ti);
        }

        void from(const std::tuple<Ts...> &tpl) {
            auto flattened = flatten(tpl);
            tuple_for_each(flattened, [&](const auto &v, size_t) {
                const cpx::TagInfo ti  = proto::protobuf::get_tag_info(v);
                const auto        &val = detail::get_underlying_value(v);
                using T                = std::decay_t<decltype(val)>;

                if (ti.field_number > 0)
                    Serialize<google::protobuf::io::CodedOutputStream, T>{doc}.from(val, ti);
            });
        }
    };

    template <typename... Ts>
    struct Deserialize<google::protobuf::io::CodedInputStream, std::tuple<Ts...>>
        : public proto::protobuf::detail::DeserializeDispatcherFor<std::tuple<Ts...>> {

        using proto::protobuf::detail::DeserializeDispatcherFor<std::tuple<Ts...>>::DeserializeDispatcherFor;
        ~Deserialize() override = default;

        void read() override {
            if (this->is_root)
                return _read(this->doc);

            std::string                            buffer = cpx::proto::protobuf::detail::read_message(*this);
            google::protobuf::io::ArrayInputStream ais(buffer.data(), (int)buffer.size());
            google::protobuf::io::CodedInputStream doc(&ais);
            return _read(doc);
        }

        void _read(google::protobuf::io::CodedInputStream &doc) {
            auto             flattened = flatten(*this->val);
            constexpr size_t size      = std::tuple_size_v<decltype(flattened)>;

            std::array<std::unique_ptr<proto::protobuf::detail::DeserializeDispatcher>, size> des = {};
            std::array<cpx::TagInfo, size>                                                    tis = {};

            tuple_for_each(flattened, [&](auto &v, size_t i) {
                auto &val        = detail::get_underlying_value(v);
                using T          = std::decay_t<decltype(val)>;
                cpx::TagInfo &ti = tis[i] = proto::protobuf::get_tag_info(v);
                if (ti.field_number > 0)
                    des[i] = std::make_unique<cpx::serde::Deserialize<google::protobuf::io::CodedInputStream, T>>(doc, val);
            });

            auto get_index = [&](int field_number) {
                size_t i = 0;
                for (; i < tis.size(); i++)
                    if (tis[i].field_number == field_number)
                        break;
                return i;
            };

            while (const uint32_t tag = doc.ReadTag()) {
                const uint32_t field_number = tag >> 3;
                const auto     wire_type    = static_cast<google::protobuf::internal::WireFormatLite::WireType>(tag & 0x07);
                const size_t   index        = get_index(field_number);

                try {
                    if (index >= sizeof...(Ts) || !des[index]) {
                        if (!google::protobuf::internal::WireFormatLite::SkipField(&doc, tag))
                            throw serde::error("cannot skip field");
                        continue;
                    }

                    auto &ser = *des[index];
                    ser.set_wire_type(wire_type)
                        .set_fixed(tis[index].fixed)
                        .set_zigzag(tis[index].zigzag)
                        .set_packed(tis[index].packed)
                        .read_len()
                        .read();
                } catch (serde::error &e) {
                    if (index >= sizeof...(Ts))
                        e.add_context(std::to_string(field_number));
                    else
                        e.add_context(tis[index].key);
                    throw;
                }
            }
        }
    };

    // reflect
    template <typename T>
    struct Serialize<google::protobuf::io::CodedOutputStream, T, std::enable_if_t<cpx::proto::protobuf::has_reflect_v<T>>> {
        google::protobuf::io::CodedOutputStream &doc;

        void from(const T &v, const cpx::TagInfo &ti) {
            return Serialize<google::protobuf::io::CodedOutputStream, cpx::proto::protobuf::const_reflect_t<T>>{doc}.from(
                cpx::proto::protobuf::reflect_of(v), ti
            );
        }

        void from(const T &v) {
            return Serialize<google::protobuf::io::CodedOutputStream, cpx::proto::protobuf::const_reflect_t<T>>{doc}.from(
                cpx::proto::protobuf::reflect_of(v)
            );
        }
    };

    template <typename T>
    struct Deserialize<google::protobuf::io::CodedInputStream, T, std::enable_if_t<cpx::proto::protobuf::has_reflect_v<T>>>
        : public proto::protobuf::detail::DeserializeDispatcherFor<T> {

        using proto::protobuf::detail::DeserializeDispatcherFor<T>::DeserializeDispatcherFor;
        ~Deserialize() override = default;

        void read() override {
            decltype(auto) proxy = cpx::proto::protobuf::reflect_of(*this->val);

            Deserialize<google::protobuf::io::CodedInputStream, cpx::proto::protobuf::reflect_t<T>> des(this->doc);
            des.inherit(*this);
            des.into(proxy);
        }
    };

    // map
    template <typename K, typename T, typename H, typename P, typename A>
    struct Serialize<
        google::protobuf::io::CodedOutputStream,
        std::unordered_map<K, T, H, P, A>,
        std::enable_if_t<
            is_serializable_v<google::protobuf::io::CodedOutputStream, K> &&
            is_serializable_v<google::protobuf::io::CodedOutputStream, T>>> {
        google::protobuf::io::CodedOutputStream &doc;

        void from(const std::unordered_map<K, T, H, P, A> &v, const cpx::TagInfo &ti) {
            std::string buffer = cpx::proto::protobuf::detail::to_message(v);
            Serialize<google::protobuf::io::CodedOutputStream, std::string>{this->doc}.from(buffer, ti);
        }

        void from(const std::unordered_map<K, T, H, P, A> &map) {
            for (auto &[k, v] : map) {
                Serialize<google::protobuf::io::CodedOutputStream, K>{this->doc}.from(k, "1");
                Serialize<google::protobuf::io::CodedOutputStream, T>{this->doc}.from(v, "2");
            }
        }
    };

    template <typename K, typename T, typename H, typename P, typename A>
    struct Deserialize<
        google::protobuf::io::CodedOutputStream,
        std::unordered_map<K, T, H, P, A>,
        std::enable_if_t<
            is_deserializable_v<google::protobuf::io::CodedInputStream, K> &&
            is_deserializable_v<google::protobuf::io::CodedInputStream, T>>>
        : public proto::protobuf::detail::DeserializeDispatcherFor<std::unordered_map<K, T, H, P, A>> {

        using proto::protobuf::detail::DeserializeDispatcherFor<std::unordered_map<K, T, H, P, A>>::DeserializeDispatcherFor;
        ~Deserialize() override = default;

        void read() override {
            if (this->is_root)
                return _read(this->doc);

            std::string                            buffer = cpx::proto::protobuf::detail::read_message(*this);
            google::protobuf::io::ArrayInputStream ais(buffer.data(), (int)buffer.size());
            google::protobuf::io::CodedInputStream doc(&ais);
            return _read(doc);
        }

        void _read(google::protobuf::io::CodedInputStream &doc) {
            serde::Deserialize<google::protobuf::io::CodedInputStream, K> dk(doc);
            serde::Deserialize<google::protobuf::io::CodedInputStream, T> dv(doc);

            while (const uint32_t tag = doc.ReadTag()) {
                const uint32_t field_number = tag >> 3;
                const auto     wire_type    = static_cast<google::protobuf::internal::WireFormatLite::WireType>(tag & 0x07);

                K k;
                T v;
                try {
                    if (field_number == 1) {
                        dk.set_wire_type(wire_type).set_fixed(false).set_zigzag(false).set_packed(true).read_len();
                        dk.into(k);
                    } else if (field_number == 2) {
                        dv.set_wire_type(wire_type).set_fixed(false).set_zigzag(false).set_packed(true).read_len();
                        dv.into(v);
                    } else {
                        if (!google::protobuf::internal::WireFormatLite::SkipField(&doc, tag))
                            throw serde::error("cannot skip field");
                        continue;
                    }
                } catch (serde::error &e) {
                    e.add_context(std::to_string(field_number));
                    throw;
                }
                this->val->emplace(std::move(k), std::move(v));
            }
        }
    };

    // time
    template <>
    struct Serialize<google::protobuf::io::CodedOutputStream, std::timespec> {
        google::protobuf::io::CodedOutputStream &doc;

        void from(const std::timespec &v, const cpx::TagInfo &ti) {
            std::string buffer = cpx::proto::protobuf::detail::to_message(v);
            Serialize<google::protobuf::io::CodedOutputStream, std::string>{this->doc}.from(buffer, ti);
        }

        void from(const std::timespec &v) {
            Serialize<google::protobuf::io::CodedOutputStream, time_t>{this->doc}.from(v.tv_sec, "1");
            Serialize<google::protobuf::io::CodedOutputStream, time_t>{this->doc}.from(v.tv_nsec, "2");
        }
    };

    template <>
    struct Deserialize<google::protobuf::io::CodedOutputStream, std::timespec>
        : public proto::protobuf::detail::DeserializeDispatcherFor<std::timespec> {

        using proto::protobuf::detail::DeserializeDispatcherFor<timespec>::DeserializeDispatcherFor;
        ~Deserialize() override = default;

        void read() override {
            if (this->is_root)
                return _read(this->doc);

            std::string                            buffer = cpx::proto::protobuf::detail::read_message(*this);
            google::protobuf::io::ArrayInputStream ais(buffer.data(), (int)buffer.size());
            google::protobuf::io::CodedInputStream doc(&ais);
            return _read(doc);
        }

        void _read(google::protobuf::io::CodedInputStream &doc) {
            const cpx::TagInfo tag_sec  = "1";
            const cpx::TagInfo tag_nsec = "2";
            auto tpl = std::make_tuple(cpx::tag_tie(this->val->tv_sec, tag_sec), cpx::tag_tie(this->val->tv_nsec, tag_nsec));

            Deserialize<google::protobuf::io::CodedInputStream, decltype(tpl)> des(doc);
            des.set_root();
            des.into(tpl);
        }
    };

    template <>
    struct Serialize<google::protobuf::io::CodedOutputStream, std::tm> {
        google::protobuf::io::CodedOutputStream &doc;

        void from(const std::tm &v, const cpx::TagInfo &ti) {
            std::string buffer = cpx::proto::protobuf::detail::to_message(v);
            Serialize<google::protobuf::io::CodedOutputStream, std::string>{this->doc}.from(buffer, ti);
        }

        void from(const std::tm &v) {
            std::tm tm  = v;
            auto    sec = timegm(&tm);
            Serialize<google::protobuf::io::CodedOutputStream, time_t>{this->doc}.from(sec, "1");
        }
    };

    template <>
    struct Deserialize<google::protobuf::io::CodedOutputStream, std::tm>
        : public proto::protobuf::detail::DeserializeDispatcherFor<std::tm> {

        using proto::protobuf::detail::DeserializeDispatcherFor<tm>::DeserializeDispatcherFor;
        ~Deserialize() override = default;

        void read() override {
            if (this->is_root)
                return _read(this->doc);

            std::string                            buffer = cpx::proto::protobuf::detail::read_message(*this);
            google::protobuf::io::ArrayInputStream ais(buffer.data(), (int)buffer.size());
            google::protobuf::io::CodedInputStream doc(&ais);
            return _read(doc);
        }

        void _read(google::protobuf::io::CodedInputStream &doc) {
            time_t             time    = timegm(this->val);
            const cpx::TagInfo tag_sec = "1";
            auto               tpl     = std::make_tuple(cpx::tag_tie(time, tag_sec));

            Deserialize<google::protobuf::io::CodedInputStream, decltype(tpl)> des(doc);
            des.set_root();
            des.into(tpl);
            *this->val = *std::gmtime(&time);
        }
    };
} // namespace cpx::serde

namespace cpx::proto::protobuf {
    template <typename T>
    [[nodiscard]]
    std::string dump(const T &val) {
        return Dump<std::string>{}.from(val);
    }

    template <typename T>
    void dump(std::ostream &os, const T &val) {
        Dump<std::ostream>{os}.from(val);
    }

    template <typename T>
    void parse(const std::string &str, T &val) {
        Parse<std::string>{str}.into(val);
    }

    template <typename T>
    [[nodiscard]]
    T parse(const std::string &str) {
        T val;
        Parse<std::string>{str}.into(val);
        return val;
    }

    template <typename T>
    void parse(std::istream &is, T &val) {
        Parse<std::istream>{is}.into(val);
    }

    template <typename T>
    [[nodiscard]]
    T parse(std::istream &is) {
        T val;
        Parse<std::istream>{is}.into(val);
        return val;
    }
} // namespace cpx::proto::protobuf

namespace cpx::proto::protobuf::detail {
    template <typename T>
    std::string to_message(const T &v) {
        std::string buffer;
        {
            google::protobuf::io::StringOutputStream os(&buffer);
            google::protobuf::io::CodedOutputStream  doc(&os);
            cpx::serde::Serialize<google::protobuf::io::CodedOutputStream, T>{doc}.from(v);
        }

        return buffer;
    }

    template <typename T>
    std::string read_message(DeserializeDispatcherFor<T> &d) {
        auto buffer = std::string();
        auto des    = cpx::serde::Deserialize<google::protobuf::io::CodedInputStream, std::string>(d.doc);
        des.inherit(d);
        des.into(buffer);
        return buffer;
    }
} // namespace cpx::proto::protobuf::detail

namespace cpx::proto::protobuf {
    inline constexpr class IO {
        friend cpx::proto::protobuf::Dump<std::ostream> operator<<(std::ostream &os, const IO &) {
            return {os};
        }

        friend cpx::proto::protobuf::Parse<std::istream> operator>>(std::istream &is, const IO &) {
            return {is};
        }
    } io;
} // namespace cpx::proto::protobuf

#endif
