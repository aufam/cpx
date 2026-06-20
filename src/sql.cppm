module;

#include <cpx/tuple.h>
#include <optional>
#include <string>
#include <vector>
#include <utility>

#ifndef BOOST_PFR_HPP
#    if __has_include(<boost/pfr.hpp>)
#        include <boost/pfr.hpp>
#    endif
#endif

export module cpx.sql;
import cpx;

#undef CPX_EXPORT
#define CPX_EXPORT export

extern "C++" {
#include "cpx/sql/sql.h"
}
