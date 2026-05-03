module;

#include <cpx/proto/protobuf.h>

export module cpx.protobuf;
import cpx;

export namespace cpx::proto::protobuf {
    using ::cpx::proto::protobuf::Deserialize;
    using ::cpx::proto::protobuf::dump;
    using ::cpx::proto::protobuf::Dump;
    using ::cpx::proto::protobuf::get_tag_info;
    using ::cpx::proto::protobuf::parse;
    using ::cpx::proto::protobuf::Parse;
    using ::cpx::proto::protobuf::Serialize;
    using ::cpx::proto::protobuf::TagInfo;
} // namespace cpx::proto::protobuf

export namespace cpx {
    namespace protobuf = ::cpx::proto::protobuf;
}

export {
    template <>
    struct cpx::serde::Dump<google::protobuf::io::CodedOutputStream, std::string>;

    template <>
    struct cpx::serde::Parse<google::protobuf::io::CodedInputStream, std::string>;

    template <typename T>
    struct cpx::serde::Serialize<
        google::protobuf::io::CodedOutputStream,
        T,
        std::enable_if_t<cpx::proto::protobuf::detail::is_numeric<T>::value>>;

    template <typename T>
    struct cpx::serde::Deserialize<
        google::protobuf::io::CodedInputStream,
        T,
        std::enable_if_t<cpx::proto::protobuf::detail::is_numeric<T>::value>>;

    template <typename T>
    struct cpx::serde::
        Serialize<google::protobuf::io::CodedOutputStream, T, std::enable_if_t<cpx::proto::protobuf::detail::is_bytes<T>::value>>;

    template <typename T>
    struct cpx::serde::Deserialize<
        google::protobuf::io::CodedInputStream,
        T,
        std::enable_if_t<cpx::proto::protobuf::detail::is_bytes<T>::value>>;

    template <typename T>
    struct cpx::serde::Serialize<
        google::protobuf::io::CodedOutputStream,
        T,
        std::enable_if_t<cpx::proto::protobuf::detail::is_repeated_numeric<T>::value>>;

    template <typename T>
    struct cpx::serde::Deserialize<
        google::protobuf::io::CodedInputStream,
        T,
        std::enable_if_t<cpx::proto::protobuf::detail::is_repeated_numeric<T>::value>>;

    template <typename T>
    struct cpx::serde::Serialize<
        google::protobuf::io::CodedOutputStream,
        std::optional<T>,
        std::enable_if_t<cpx::serde::is_serializable_v<google::protobuf::io::CodedOutputStream, T>>>;

    template <typename T>
    struct cpx::serde::Deserialize<
        google::protobuf::io::CodedInputStream,
        std::optional<T>,
        std::enable_if_t<cpx::serde::is_deserializable_v<google::protobuf::io::CodedInputStream, T>>>;

    template <typename T>
    struct cpx::serde::Serialize<
        google::protobuf::io::CodedOutputStream,
        T,
        std::enable_if_t<cpx::proto::protobuf::detail::is_repeated_serializable<T>::value>>;

    template <typename T>
    struct cpx::serde::Deserialize<
        google::protobuf::io::CodedInputStream,
        T,
        std::enable_if_t<cpx::proto::protobuf::detail::is_repeated_deserializable<T>::value>>;

    template <typename T>
    struct cpx::serde::Serialize<
        google::protobuf::io::CodedOutputStream,
        T,
        std::enable_if_t<cpx::proto::protobuf::detail::is_message<T>::value>>;

    template <typename T>
    struct cpx::serde::Deserialize<
        google::protobuf::io::CodedInputStream,
        T,
        std::enable_if_t<cpx::proto::protobuf::detail::is_message<T>::value>>;

#ifdef BOOST_PFR_HPP
    template <typename S>
    struct cpx::serde::Serialize<
        google::protobuf::io::CodedOutputStream,
        S,
        std::enable_if_t<std::is_aggregate_v<S> && !cpx::proto::protobuf::detail::is_repeated<S>::value>>;

    template <typename S>
    struct cpx::serde::Deserialize<
        google::protobuf::io::CodedInputStream,
        S,
        std::enable_if_t<std::is_aggregate_v<S> && !cpx::proto::protobuf::detail::is_repeated<S>::value>>;
#endif

    template <typename K, typename T, typename H, typename P, typename A>
    struct cpx::serde::Serialize<
        google::protobuf::io::CodedOutputStream,
        std::unordered_map<K, T, H, P, A>,
        std::enable_if_t<
            cpx::serde::is_serializable_v<google::protobuf::io::CodedOutputStream, K> &&
            cpx::serde::is_serializable_v<google::protobuf::io::CodedOutputStream, T>>>;

    template <typename K, typename T, typename H, typename P, typename A>
    struct cpx::serde::Deserialize<
        google::protobuf::io::CodedOutputStream,
        std::unordered_map<K, T, H, P, A>,
        std::enable_if_t<
            cpx::serde::is_deserializable_v<google::protobuf::io::CodedInputStream, K> &&
            cpx::serde::is_deserializable_v<google::protobuf::io::CodedInputStream, T>>>;
} // namespace cpx::serde
