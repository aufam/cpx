#include <cpx/fmt.h>
#include <cpx/toml/marzer_toml.h>

int main() {
    enum class Level { High = 10, Medium, Low };

    static_assert(cpx::detail::has_reflect_traits<cpx::WeakReflect, Level>::value == true);
    static_assert(cpx::has_reflect_v<Level> == true);
    static_assert(cpx::fmt::has_reflect_v<Level> == true);

    Level       level = Level::High;
    std::string text  = "Level is high";

    std::tuple data = {
        cpx::field_ref(level) = "level",
        cpx::field_ref(text)  = "text",
    };

    fmt::println("=== toml ===");
    fmt::println("{}", cpx::toml::marzer_toml::dump(data));

    fmt::println("=== fmt ===");
    fmt::println("{}", data);

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
