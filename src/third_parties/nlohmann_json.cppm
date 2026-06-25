module;

#include <cpx/json/json.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <cpx/module.h>

#include <variant>
#include <nlohmann/json.hpp>

export module cpx.nlohmann_json;
export import cpx.json;

extern "C++" {
#include "cpx/json/nlohmann_json.h"
}
