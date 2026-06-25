module;

#include <cpx/reflect.h>
#include <cpx/module.h>

export module cpx.yaml;
import cpx;

extern "C++" {
#include "cpx/yaml/yaml.h"
}
