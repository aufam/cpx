module;

#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/module.h>

#include <optional>
#include <variant>

#include <msgpack.hpp>

export module cpx.msgpack;
import cpx;

extern "C++" {
#include "cpx/msgpack.h"
}
