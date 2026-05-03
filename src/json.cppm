module;

#include <cpx/json/json.h>

export module cpx.json;
import cpx;

export namespace cpx::json {
    using ::cpx::json::get_tag_info;
    using ::cpx::json::get_tag_info_from_tuple;
} // namespace cpx::json
