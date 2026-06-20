module;

#include <cpx/reflect.h>

export module cpx.cli;
import cpx;

#undef CPX_EXPORT
#define CPX_EXPORT export

extern "C++" {
#include "cpx/cli/cli.h"
}
