module;

#include <cpx/toml/toml.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <array>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

#include <toml.hpp>

export module cpx.toruniina_toml;
export import cpx.toml;

#undef CPX_EXPORT
#define CPX_EXPORT export

extern "C++" {
#include "cpx/toml/toruniina_toml.h"
}
