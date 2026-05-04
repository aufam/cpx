module;

#define yyjson_api_inline inline
#include <cpx/json/yy_json.h>

export module cpx.yyjson;
import cpx;
export import cpx.serde;
export import cpx.json;

export {
    using ::yyjson_doc;
    using ::yyjson_mut_doc;
    using ::yyjson_mut_val;
    using ::yyjson_val;
}

export namespace cpx::json::yy_json {
    using ::cpx::json::yy_json::Deserialize;
    using ::cpx::json::yy_json::dump;
    using ::cpx::json::yy_json::Dump;
    using ::cpx::json::yy_json::parse;
    using ::cpx::json::yy_json::Parse;
    using ::cpx::json::yy_json::parse_from_file;
    using ::cpx::json::yy_json::Serialize;
} // namespace cpx::json::yy_json

export namespace cpx {
    namespace yyjson = ::cpx::json::yy_json;
}
