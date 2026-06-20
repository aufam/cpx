module;

#include <cpx/reflect.h>

export module cpx.json;
import cpx;

#undef CPX_EXPORT
#define CPX_EXPORT export

extern "C++" {
#include "cpx/json/json.h"
}
