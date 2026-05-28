module;

#include <cpx/yaml/yaml.h>

export module cpx.yaml;
import cpx;

export namespace cpx::yaml {
    using ::cpx::yaml::const_reflect_t;
    using ::cpx::yaml::get_tag_info;
    using ::cpx::yaml::has_reflect;
    using ::cpx::yaml::has_reflect_v;
    using ::cpx::yaml::Reflect;
    using ::cpx::yaml::reflect_of;
    using ::cpx::yaml::reflect_t;
} // namespace cpx::yaml
