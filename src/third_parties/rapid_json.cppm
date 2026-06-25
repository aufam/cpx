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
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#if __has_include(<rapidjson/ostreamwrapper.h>)
#    include <rapidjson/ostreamwrapper.h>
#endif
#if __has_include(<rapidjson/istreamwrapper.h>)
#    include <rapidjson/istreamwrapper.h>
#endif
#include <rapidjson/error/en.h>

export module cpx.rapid_json;
export import cpx.json;

extern "C++" {
#include "cpx/json/rapid_json.h"
}
