#ifndef CPX_TOML_TOML_H
#define CPX_TOML_TOML_H

#include <cpx/tag_info.h>

namespace cpx::toml {
    template <typename T>
    constexpr TagInfo get_tag_info(const T &field) {
        return get_tag_info(field, "toml");
    }

    template <typename... T>
    constexpr TagInfoTuple<sizeof...(T)> get_tag_info_from_tuple(const std::tuple<T...> &fields) {
        return get_tag_info_from_tuple(fields, "toml");
    }
} // namespace cpx::toml

#endif
