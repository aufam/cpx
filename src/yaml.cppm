module;

#include <cpx/yaml/yaml.h>

export module cpx.yaml;
import cpx;

export namespace cpx::yaml {
    using ::cpx::yaml::get_tag_info;
    using ::cpx::yaml::get_tag_info_from_tuple;
} // namespace cpx::yaml
