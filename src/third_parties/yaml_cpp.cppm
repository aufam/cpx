module;

#include <cpx/yaml/jbeder_yaml.h>

export module cpx.yaml_cpp;
import cpx;
export import cpx.serde;
export import cpx.yaml;

export namespace cpx::yaml::jbeder_yaml {
    using ::cpx::yaml::jbeder_yaml::Deserialize;
    using ::cpx::yaml::jbeder_yaml::dump;
    using ::cpx::yaml::jbeder_yaml::Dump;
    using ::cpx::yaml::jbeder_yaml::dump_to_stream;
    using ::cpx::yaml::jbeder_yaml::parse;
    using ::cpx::yaml::jbeder_yaml::Parse;
    using ::cpx::yaml::jbeder_yaml::parse_from_file;
    using ::cpx::yaml::jbeder_yaml::Serialize;
} // namespace cpx::yaml::jbeder_yaml

export namespace cpx {
    namespace yaml_cpp = ::cpx::yaml::jbeder_yaml;
}
