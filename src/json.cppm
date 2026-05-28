module;

#include <cpx/json/json.h>

export module cpx.json;
import cpx;

export namespace cpx::json {
    using ::cpx::json::const_reflect_t;
    using ::cpx::json::get_tag_info;
    using ::cpx::json::has_reflect;
    using ::cpx::json::has_reflect_v;
    using ::cpx::json::Reflect;
    using ::cpx::json::reflect_of;
    using ::cpx::json::reflect_t;
} // namespace cpx::json
