module;

#include <cpx/cli/cli.h>
#include <cpx/serde/deserialize.h>
#include <cpx/serde/error.h>
#include <cpx/extend.h>
#include <cpx/reflect_builtin.h>
#include <variant>

#include <CLI/CLI.hpp>

export module cpx.cli11;
export import cpx.cli;

#undef CPX_EXPORT
#define CPX_EXPORT export

extern "C++" {
#include "cpx/cli/cli11.h"
}
