module;

#include <cpx/toml/toml.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <cpx/module.h>

#include <array>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

#include <toml++/toml.h>

export module cpx.marzer_toml;
export import cpx.toml;

extern "C++" {
#include "cpx/toml/marzer_toml.h"
}
