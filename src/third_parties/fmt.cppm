module;

#include <cpx/fmt_reflect.h>
#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <cpx/module.h>
#include <fmt/ranges.h>
#include <fmt/chrono.h>
#include <optional>
#include <variant>

export module cpx.fmt;
import cpx;

extern "C++" {
#include "cpx/fmt.h"
}
