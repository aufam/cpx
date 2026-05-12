#ifndef CPX_JSON_JSON_H
#define CPX_JSON_JSON_H

#include <cpx/tag_info.h>

namespace cpx::json {
    template <typename T>
    constexpr TagInfo get_tag_info(const T &field) {
        if constexpr (::cpx::detail::is_tag_info_for_v<T>)
            return field.ti;
        else
            return ::cpx::get_tag_info(field, "json");
    }

    template <typename... T>
    constexpr TagInfoTuple<sizeof...(T)> get_tag_info_from_tuple(const std::tuple<T...> &fields) {
        TagInfoTuple<sizeof...(T)> ts       = {};
        bool                       is_array = sizeof...(T) > 0;

        tuple_for_each(fields, [&](const auto &field, size_t i) {
            TagInfo &t = ts.ts[i] = ::cpx::json::get_tag_info(field);
            is_array &= t.positional;
        });

        ts.is_obj = !is_array;
        return ts;
    }
} // namespace cpx::json

#endif
