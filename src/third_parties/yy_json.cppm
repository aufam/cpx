module;

#include <cpx/json/json.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <cpx/defer.h>
#include <cpx/module.h>

#include <array>
#include <variant>
#include <vector>
#include <tuple>
#include <unordered_map>

#define yyjson_api_inline inline
#include <yyjson.h>

export module cpx.yy_json;
export import cpx.json;

extern "C++" {
#include "cpx/json/yy_json.h"
}
