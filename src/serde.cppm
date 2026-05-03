module;

#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/serde/error.h>

export module cpx.serde;

export namespace cpx::serde {
    using ::cpx::serde::Dump;
    using ::cpx::serde::is_serializable;
    using ::cpx::serde::is_serializable_v;
    using ::cpx::serde::Serialize;

    using ::cpx::serde::Deserialize;
    using ::cpx::serde::is_deserializable;
    using ::cpx::serde::is_deserializable_v;
    using ::cpx::serde::Parse;

    using ::cpx::serde::error;
    using ::cpx::serde::size_mismatch_error;
    using ::cpx::serde::type_mismatch_error;
} // namespace cpx::serde
