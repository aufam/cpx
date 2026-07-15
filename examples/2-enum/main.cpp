#include <cpx/fmt.h>
#include <cpx/toml/marzer_toml.h>

template <typename T>
using Tag = cpx::Tag<T>;

int main() {
    enum class Level { High = 10, Medium, Low };

    static_assert(cpx::detail::has_reflect_traits<cpx::SelfReflect, void>::value == false);
    static_assert(cpx::detail::has_reflect_traits<cpx::WeakReflect, Level>::value == true);
    static_assert(cpx::has_reflect_v<Level> == true);
    static_assert(cpx::fmt::has_reflect_v<Level> == true);
    static_assert(fmt::is_formattable<Tag<Level>>::value == true);

    std::tuple data = {
        Tag<Level>{"fmt,toml:`level`"},
        Tag<std::string>{"fmt,toml:`text`"},
    };

    // access the field using operator()
    std::get<0>(data)() = Level::High;
    std::get<1>(data)() = "Level is high";

    fmt::println("=== toml ===");
    fmt::println("{}", cpx::toml::marzer_toml::dump(data));

    fmt::println("=== fmt ===");
    fmt::println("{}", std::get<0>(data));
    fmt::println("{:?}", std::get<1>(data));

    const char *toml_doc = R"toml(
        text = "From toml"
        level = "Unknown"
    )toml";

    fmt::println("=== invalid value ===");
    try {
        cpx::toml::marzer_toml::parse(toml_doc, data);
    } catch (cpx::serde::error &e) {
        fmt::println("{}", e.what());
    }
}
