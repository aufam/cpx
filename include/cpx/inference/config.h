#ifndef CPX_INFERENCE_CONFIG_H
#define CPX_INFERENCE_CONFIG_H

#include <cpx/tag_info.h>
#include <string>
#include <optional>

namespace cpx::inference {
    CPX_EXPORT struct Config {
        unsigned char intra_threads = 0;
        unsigned char inter_threads = 0;
        std::string   gpu_mem_limit;

        struct Ov {
            enum class Optimization { Default, Balanced, HighPerformance, LowLatency };
            enum class ExecutionMode { Default, Accuracy, Performance };

            std::string         device = "CPU"; // "CPU", "GPU", "NPU", etc.
            std::string         device_id;
            std::optional<bool> enable_cpu_pinning;
            std::optional<bool> allow_auto_batch;
            Optimization        optimization            = Optimization::Default;
            ExecutionMode       prefered_execution_mode = ExecutionMode::Default;

            static constexpr std::tuple __field_tags__ = {
                cpx::field<&Ov::device>             = "device,skipmissing",
                cpx::field<&Ov::device_id>          = "device-id,skipmissing,omitempty",
                cpx::field<&Ov::enable_cpu_pinning> = "enable-cpu-pinning,skipmissing",
                cpx::field<&Ov::allow_auto_batch>   = "allow-auto-batch,skipmissing",
            };
        } ov;

        struct Ort {
            enum class GraphOptimizationLevel {
                ORT_DISABLE_ALL,
                ORT_ENABLE_BASIC,
                ORT_ENABLE_EXTENDED,
                ORT_ENABLE_LAYOUT,
                ORT_ENABLE_ALL = 99
            };

            bool                   use_cuda                 = false;
            unsigned               device_id                = 0;
            bool                   enable_cpu_mem_arena     = true;
            bool                   verbose                  = false;
            GraphOptimizationLevel graph_optimization_level = GraphOptimizationLevel::ORT_ENABLE_ALL;

            static constexpr std::tuple __field_tags__ = {
                cpx::field<&Ort::use_cuda>                 = "use-cuda,skipmissing",
                cpx::field<&Ort::device_id>                = "device-id,skipmissing,omitempty",
                cpx::field<&Ort::enable_cpu_mem_arena>     = "enable-cpu-mem-arena,skipmissing",
                cpx::field<&Ort::verbose>                  = "verbose,skipmissing",
                cpx::field<&Ort::graph_optimization_level> = "graph-optimization-level,skipmissing",
            };
        } ort;

        static size_t parse_mem_limit(const std::string &s) {
            size_t num  = std::stoull(s);
            auto   unit = s.back();
            if (unit == 'g' || unit == 'G')
                return num * 1024ULL * 1024ULL * 1024ULL;
            if (unit == 'm' || unit == 'M')
                return num * 1024ULL * 1024ULL;
            if (unit == 'k' || unit == 'K')
                return num * 1024ULL;
            return num;
        }
    };
} // namespace cpx::inference

#endif
