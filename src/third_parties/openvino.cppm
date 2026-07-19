module;

#include <openvino/openvino.hpp>
#include <cpx/inference/runtime.h>
#include <cpx/module.h>

export module cpx.inference.openvino;
import cpx.inference;

extern "C++" {
#include "cpx/inference/openvino.h"
}
