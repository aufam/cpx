module;

#include <cpx/json/nlohmann_json.h>

export module cpx.nlohmann.json;
import cpx;
export import cpx.serde;
export import cpx.json;

export namespace cpx::json::nlohmann_json {
    using ::cpx::json::nlohmann_json::Deserialize;
    using ::cpx::json::nlohmann_json::dump;
    using ::cpx::json::nlohmann_json::Dump;
    using ::cpx::json::nlohmann_json::parse;
    using ::cpx::json::nlohmann_json::Parse;
    using ::cpx::json::nlohmann_json::Serialize;
} // namespace cpx::json::nlohmann_json

export namespace cpx {
    namespace nlohmann_json = ::cpx::json::nlohmann_json;
}

// export import nlohmann.json;
export namespace nlohmann {
    using ::nlohmann::adl_serializer;
    using ::nlohmann::basic_json;
    using ::nlohmann::json;
    using ::nlohmann::json_pointer;
    using ::nlohmann::ordered_json;
    using ::nlohmann::ordered_map;
    using ::nlohmann::to_string;
} // namespace nlohmann
