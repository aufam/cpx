#ifndef CPX_JSON_JSON_H
#define CPX_JSON_JSON_H

#include <cpx/tag_info.h>

namespace cpx::json {
    template <typename T>
    constexpr TagInfo get_tag_info(const T &field) {
        return get_tag_info(field, "json");
    }

    template <typename... T>
    constexpr TagInfoTuple<sizeof...(T)> get_tag_info_from_tuple(const std::tuple<T...> &fields) {
        return get_tag_info_from_tuple(fields, "json");
    }
} // namespace cpx::json

#endif
