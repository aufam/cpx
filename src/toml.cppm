module;

#include <cpx/toml/toml.h>

export module cpx.toml;
import cpx;

export namespace cpx::toml {
    using ::cpx::toml::get_tag_info;
    using ::cpx::toml::get_tag_info_from_tuple;
} // namespace cpx::toml
