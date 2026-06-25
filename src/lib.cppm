module;

#include <cstdlib>
#include <ctime>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>

#if __has_include(<boost/pfr.hpp>)
#    include <boost/pfr.hpp>
#endif

#if __has_include(<magic_enum/magic_enum.hpp>)
#    include <magic_enum/magic_enum.hpp>
#endif

#include <cpx/serde/error.h>
#include <cpx/module.h>

export module cpx;

extern "C++" {
#include "cpx/tuple.h"
#include "cpx/optional.h"
#include "cpx/defer.h"
#include "cpx/overload.h"
#include "cpx/result.h"
#include "cpx/tag.h"
#include "cpx/time.h"

#include "cpx/iter.h"
#include "cpx/extend.h"
#include "cpx/tag_info.h"
#include "cpx/reflect.h"
#include "cpx/reflect_builtin.h"
}
