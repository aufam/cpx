module;

#include <type_traits>
#include <exception>
#include <string>

export module cpx.serde;

#undef CPX_EXPORT
#define CPX_EXPORT export

extern "C++" {
#include "cpx/serde/serialize.h"
#include "cpx/serde/deserialize.h"
#include "cpx/serde/error.h"
}
