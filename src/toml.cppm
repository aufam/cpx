module;

#include <cpx/reflect.h>
#include <cpx/time.h>
#include <cpx/module.h>

export module cpx.toml;
import cpx;

extern "C++" {
#include "cpx/toml/toml.h"
}
