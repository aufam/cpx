#include <cpx/toml/toruniina_toml.h>
#include <gtest/gtest.h>

struct Package {
    std::string              name;
    std::string              version;
    int                      edition = 17;
    std::vector<std::string> authors;
    std::string              description;
    std::string              license;

    static constexpr auto __field_tags__ = std::make_tuple(
        cpx::field<&Package::name>        = "name",
        cpx::field<&Package::version>     = "version,skipmissing",
        cpx::field<&Package::edition>     = "edition,skipmissing",
        cpx::field<&Package::authors>     = "authors,skipmissing",
        cpx::field<&Package::description> = "description,skipmissing",
        cpx::field<&Package::license>     = "license,skipmissing"
    );
};

struct Carton {
    Package                                                   package;
    std::unordered_map<std::string, std::vector<std::string>> features;
    bool                                                      no_default_features;
};

template <>
struct cpx::Reflect<Carton> {
    static constexpr auto field_tags = std::make_tuple(
        cpx::field<&Carton::package>             = "package",
        cpx::field<&Carton::features>            = "features,skipmissing",
        cpx::field<&Carton::no_default_features> = "no-default-features,skipmissing"
    );
};

TEST(toml, toml11) {
    Carton c;
    cpx::toruniina_toml::parse(
        R"toml(
            [package]
            name = "fmt"
            edition = 11
            description = "String formatter"
            homepage = "https://fmt.dev"
        )toml",
        c
    );

    EXPECT_EQ(c.package.name, "fmt");
    EXPECT_EQ(c.package.edition, 11);
    EXPECT_EQ(c.package.description, "String formatter");
}
