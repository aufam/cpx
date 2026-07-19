module;

#include <cpx/tag_info.h>
#include <cpx/time.h>
#include <cpx/module.h>

#include <string>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>
#include <variant>

export module cpx.genai;
import cpx;

extern "C++" {
#include "cpx/genai/openai_common.h"
#include "cpx/genai/openai.h"
#include "cpx/genai/ollama.h"
#include "cpx/genai/anthropic.h"
#include "cpx/genai/gemini.h"
}
