module;

#include <cpx/yaml/yaml.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <array>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

#include <yaml-cpp/yaml.h>

export module cpx.jbeder_yaml;
export import cpx.yaml;

#undef CPX_EXPORT
#define CPX_EXPORT export

extern "C++" {
#include "cpx/yaml/jbeder_yaml.h"
}
