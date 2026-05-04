module;

#include <cpx/toml/toruniina_toml.h>

export module cpx.toml11;
import cpx;
export import cpx.serde;
export import cpx.toml;

export namespace cpx::toml::toruniina_toml {
    using spec = __toml11::spec;

    using ::cpx::toml::toruniina_toml::Deserialize;
    using ::cpx::toml::toruniina_toml::dump;
    using ::cpx::toml::toruniina_toml::Dump;
    using ::cpx::toml::toruniina_toml::parse;
    using ::cpx::toml::toruniina_toml::Parse;
    using ::cpx::toml::toruniina_toml::parse_from_file;
    using ::cpx::toml::toruniina_toml::Serialize;
} // namespace cpx::toml::toruniina_toml

export namespace cpx {
    namespace toml11 = ::cpx::toml::toruniina_toml;
}
