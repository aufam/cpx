module;

#include <cpx/cli/cli.h>

export module cpx.cli;
import cpx;

export namespace cpx::cli {
    using ::cpx::cli::get_tag_info;
    using ::cpx::cli::get_tag_info_from_tuple;
} // namespace cpx::cli
