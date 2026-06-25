module;

#include <optional>
#include <string>
#include <vector>
#include <utility>

#if __has_include(<boost/pfr.hpp>)
#    include <boost/pfr.hpp>
#endif

#include <cpx/tuple.h>
#include <cpx/module.h>

export module cpx.sql;
import cpx;

extern "C++" {
#include "cpx/sql/sql.h"
}
