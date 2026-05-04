module;

#include <cpx/tag_info.h>
#include <optional>
#include <variant>

#if __has_include(<boost/pfr.hpp>)
#    include <boost/pfr.hpp>
#endif

#if __has_include(<magic_enum/magic_enum.hpp>)
#    include <magic_enum/magic_enum.hpp>
#endif

#define FMT_RANGES_H_
#define FMT_CHRONO_H_

export module cpx.fmt;
export import fmt;
import cpx;

#include "cpx/fmt.h"
