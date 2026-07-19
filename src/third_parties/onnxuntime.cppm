module;

#include <cpx/inference/runtime.h>
#include <cpx/module.h>

#include <onnxruntime_cxx_api.h>
#include <unordered_map>

export module cpx.inference.onnxruntime;
import cpx.inference;

extern "C++" {
#include "cpx/inference/onnxruntime.h"
}
