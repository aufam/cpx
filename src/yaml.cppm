module;

#include <cpx/reflect.h>

export module cpx.yaml;
import cpx;

#undef CPX_EXPORT
#define CPX_EXPORT export

extern "C++" {
#include "cpx/yaml/yaml.h"
}
