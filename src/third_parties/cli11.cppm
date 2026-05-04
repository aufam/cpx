module;

#include <cpx/cli/cli11.h>

export module cpx.cli11;
import cpx;
export import cpx.cli;
export import cli11;

export namespace cpx::cli::cli11 {
    using ::cpx::cli::cli11::Deserialize;
    using ::cpx::cli::cli11::help;
    using ::cpx::cli::cli11::is_deserializable;
    using ::cpx::cli::cli11::parse;
    using ::cpx::cli::cli11::Parse;
    using ::cpx::cli::cli11::parse_with_subcommands;
} // namespace cpx::cli::cli11

export namespace cpx {
    namespace cli11 = ::cpx::cli::cli11;
}
