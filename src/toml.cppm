module;

#include <cpx/toml/toml.h>

export module cpx.toml;
import cpx;

export namespace cpx::toml {
    using ::cpx::toml::const_reflect_t;
    using ::cpx::toml::get_tag_info;
    using ::cpx::toml::has_reflect;
    using ::cpx::toml::has_reflect_v;
    using ::cpx::toml::Reflect;
    using ::cpx::toml::reflect_of;
    using ::cpx::toml::reflect_t;
} // namespace cpx::toml
