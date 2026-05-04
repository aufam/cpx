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
