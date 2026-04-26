#ifndef CPX_YAML_YAML_H
#define CPX_YAML_YAML_H

#include <cpx/tag_info.h>

namespace cpx::yaml {
    template <typename T>
    constexpr TagInfo get_tag_info(const T &field) {
        return ::cpx::get_tag_info(field, "yaml");
    }

    template <typename... T>
    constexpr TagInfoTuple<sizeof...(T)> get_tag_info_from_tuple(const std::tuple<T...> &fields) {
        return ::cpx::get_tag_info_from_tuple(fields, "yaml");
    }
} // namespace cpx::yaml

#endif
