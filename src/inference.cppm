module;

#include <cpx/tag_info.h>
#include <cpx/reflect.h>
#include <cpx/module.h>

#include <string>
#include <optional>
#include <stdexcept>
#include <vector>
#include <variant>

export module cpx.inference;
import cpx;

extern "C++" {
#include "cpx/inference/config.h"
#include "cpx/inference/tensor.h"
#include "cpx/inference/runtime.h"
#include "cpx/inference/cross_encoder.h"
#include "cpx/inference/sentence_transformers.h"
}
