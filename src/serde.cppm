module;

#include <type_traits>
#include <exception>
#include <string>
#include <cpx/module.h>
#include <cpx/nomodule.h>

export module cpx.serde;

extern "C++" {
#include "cpx/serde/serialize.h"
#include "cpx/serde/deserialize.h"
#include "cpx/serde/error.h"
}
