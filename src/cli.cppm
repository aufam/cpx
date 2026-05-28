module;

#include <cpx/cli/cli.h>

export module cpx.cli;
import cpx;

export namespace cpx::cli {
    using ::cpx::cli::const_reflect_t;
    using ::cpx::cli::get_tag_info;
    using ::cpx::cli::has_reflect;
    using ::cpx::cli::has_reflect_v;
    using ::cpx::cli::Reflect;
    using ::cpx::cli::reflect_of;
    using ::cpx::cli::reflect_t;
} // namespace cpx::cli
