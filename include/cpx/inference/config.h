#ifndef CPX_INFERENCE_CONFIG_H
#define CPX_INFERENCE_CONFIG_H

#include <cpx/tag.h>
#include <string>
#include <optional>

namespace cpx::inference {
    struct Config {
        Tag<std::optional<unsigned char>> intra_threads = "json:`intraThreads` toml,yaml:`intra-threads`";
        Tag<std::optional<unsigned char>> inter_threads = "json:`interThreads` toml,yaml:`inter-threads`";
        Tag<std::optional<std::string>>   gpu_mem_limit = "json:`gpuMemLimit`  toml,yaml:`gpu-mem-limit`";

        struct OpenVINO {
            enum class Optimization { Default, Balanced, HighPerformance, LowLatency };
            enum class ExecutionMode { Accuracy, Performance };

            Tag<std::string>                device = {"json,toml,yaml:`device,skipmissing`", "CPU"}; // "CPU", "GPU", "NPU", etc.
            Tag<std::optional<std::string>> device_id          = {"json:`deviceId`         toml,yaml:`device-id`"};
            Tag<std::optional<bool>>        enable_cpu_pinning = {"json:`enableCpuPinning` toml,yaml:`enable-cpu-pinning`"};
            Tag<std::optional<bool>>        allow_auto_batch   = {"json:`allowAutoBatch`   toml,yaml:`allow-auto-batch`"};
            Tag<Optimization>               optimization = {"json,toml,yaml:`optimization,skipmissing`", Optimization::Default};
            Tag<std::optional<ExecutionMode>> prefered_execution_mode = {
                "json:`preferedExecutionMode` toml,yaml:`prefered-execution-mode`"
            };
        };
        Tag<OpenVINO> ov = "json,toml:`ov,skipmissing`";

        struct ONNXRuntime {
            enum class GraphOptimizationLevel {
                ORT_DISABLE_ALL,
                ORT_ENABLE_BASIC,
                ORT_ENABLE_EXTENDED,
                ORT_ENABLE_LAYOUT,
                ORT_ENABLE_ALL = 99
            };

            Tag<bool>     use_cuda             = {"json:`useCuda,skipmissing`  toml,yaml:`use-cuda,skipmissing`", false};
            Tag<unsigned> device_id            = {"json:`deviceId,skipmissing` toml,yaml:`device-id,skipmissing`", 0};
            Tag<bool>     enable_cpu_mem_arena = {
                "json:`enableCpuMemArena,skipmissing` toml,yaml:`enable-cpu-mem-arena,skipmissing`", true
            };
            Tag<bool>                   verbose                  = {"json,toml,yaml:`verbose,skipmissing`", false};
            Tag<GraphOptimizationLevel> graph_optimization_level = {
                "json:`graphOptimizationLevel,skipmissing`"
                "toml,yaml:`graph-optimization-level,skipmissing`",
                GraphOptimizationLevel::ORT_ENABLE_ALL
            };
        };
        Tag<ONNXRuntime> ort = "json,toml:`ort,skipmissing`";

        // TODO
        struct TensorRT {
            bool   fp16               = false;
            bool   int8               = false;
            size_t max_workspace_size = 1ULL << 30; // 1GB default
        } trt;

        static size_t parse_mem_limit(const std::string &s) {
            size_t num  = std::stoull(s);
            auto   unit = s.back();
            if (unit == 'g' || unit == 'G')
                return num * 1024ULL * 1024ULL * 1024ULL;
            if (unit == 'm' || unit == 'm')
                return num * 1024ULL * 1024ULL;
            if (unit == 'k' || unit == 'K')
                return num * 1024ULL;
            return num;
        }
    };
} // namespace cpx::inference

#endif
