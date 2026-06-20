module;

#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <optional>
#include <variant>

#include <msgpack.hpp>

export module cpx.msgpack;
import cpx;

#undef CPX_EXPORT
#define CPX_EXPORT export
#define _CPX_MSGPACK_HPP

extern "C++" {
#include "cpx/msgpack.h"
}
