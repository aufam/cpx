#ifndef CPX_CLI_H
#define CPX_CLI_H

#include <cpx/tag_info.h>

namespace cpx::cli {
    template <typename T>
    constexpr TagInfo get_tag_info(const T &field) {
        return get_tag_info(field, "opt");
    }

    template <typename... T>
    constexpr TagInfoTuple<sizeof...(T)> get_tag_info_from_tuple(const std::tuple<T...> &fields) {
        return get_tag_info_from_tuple(fields, "opt");
    }
} // namespace cpx::cli

#endif
