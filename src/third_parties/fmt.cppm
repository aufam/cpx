module;

#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <optional>
#include <variant>
#include <chrono>

#define FMT_RANGES_H_
#define FMT_CHRONO_H_

export module cpx.fmt;
export import fmt;
import cpx;

#define CPX_MODULE
#include "cpx/fmt.h"
