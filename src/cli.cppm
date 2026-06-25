module;

#include <cpx/reflect.h>
#include <cpx/module.h>

export module cpx.cli;
import cpx;

extern "C++" {
#include "cpx/cli/cli.h"
}
