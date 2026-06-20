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

#include <cpx/serde/error.h>

#ifndef BOOST_PFR_HPP
#    if __has_include(<boost/pfr.hpp>)
#        include <boost/pfr.hpp>
#    endif
#endif

#ifndef NEARGYE_MAGIC_ENUM_HPP
#    if __has_include(<magic_enum/magic_enum.hpp>)
#        include <magic_enum/magic_enum.hpp>
#    endif
#endif

export module cpx;

#undef CPX_EXPORT
#define CPX_EXPORT export

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
