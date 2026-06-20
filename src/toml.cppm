module;

#include <cpx/reflect.h>
#include <cpx/time.h>

export module cpx.toml;
import cpx;

#undef CPX_EXPORT
#define CPX_EXPORT export

extern "C++" {
#include "cpx/toml/toml.h"
}
