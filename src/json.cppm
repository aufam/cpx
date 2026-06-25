module;

#include <cpx/reflect.h>
#include <cpx/module.h>

export module cpx.json;
import cpx;

extern "C++" {
#include "cpx/json/json.h"
}
