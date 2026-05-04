module;

#include <cpx/defer.h>
#include <cpx/extend.h>
#include <cpx/iter.h>
#include <cpx/optional.h>
#include <cpx/overload.h>
#include <cpx/result.h>
#include <cpx/tag.h>
#include <cpx/tag_info.h>
#include <cpx/time.h>
#include <cpx/time.h>
#include <cpx/tuple.h>

export module cpx;

export using std::tm;

export namespace cpx {
    using ::cpx::defer;

    using ::cpx::Extend;
    using ::cpx::flatten;
    using ::cpx::flatten_one;
    using ::cpx::is_extended;
    using ::cpx::is_extended_v;
    using ::cpx::remove_extend;
    using ::cpx::remove_extend_t;

    using ::cpx::Iter;
    using ::cpx::iterate;
    using ::cpx::reversed;
    using ::cpx::zip;

    using ::cpx::and_then;
    using ::cpx::is_optional;
    using ::cpx::is_optional_v;
    using ::cpx::or_else;
    using ::cpx::transform;
    using ::cpx::value_or_else;

    using ::cpx::overload;
    using ::cpx::visit;

    using ::cpx::bad_result_access;
    using ::cpx::Err;
    using ::cpx::Ok;
    using ::cpx::Result;

    using ::cpx::is_tagged;
    using ::cpx::is_tagged_v;
    using ::cpx::remove_tag;
    using ::cpx::remove_tag_t;
    using ::cpx::Tag;

    using ::cpx::get_tag_info;
    using ::cpx::get_tag_info_from_tuple;
    using ::cpx::TagInfo;
    using ::cpx::TagInfoTuple;

    using ::cpx::tm_from_string;
    using ::cpx::tm_max;
    using ::cpx::tm_min;
    using ::cpx::tm_now;
    using ::cpx::tm_to_string;

    using ::cpx::is_invocable_with_tuple;
    using ::cpx::is_invocable_with_tuple_v;
    using ::cpx::is_tuple;
    using ::cpx::is_tuple_v;
    using ::cpx::tie_if;
    using ::cpx::tie_if_one;
    using ::cpx::tuple_for_each;
} // namespace cpx
