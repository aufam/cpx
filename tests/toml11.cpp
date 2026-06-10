#include <cpx/toml/toruniina_toml.h>
#include <gtest/gtest.h>


struct Package {
    std::string              name;
    std::string              version;
    int                      edition = 17;
    std::vector<std::string> authors;
    std::string              description;
    std::string              license;
};

struct Carton {
    Package                                                   package;
    std::unordered_map<std::string, std::vector<std::string>> features;
    bool                                                      no_default_features;
};

template <>
struct cpx::Reflect<Package> //
    : Fields<
          Reflect<Package>, //
          &Package::name,
          &Package::version,
          &Package::edition,
          &Package::authors,
          &Package::description,
          &Package::license> {
    static constexpr TagInfo name        = "name";
    static constexpr TagInfo version     = "version,skipmissing";
    static constexpr TagInfo edition     = "edition,skipmissing";
    static constexpr TagInfo authors     = "authors,skipmissing";
    static constexpr TagInfo description = "description,skipmissing";
    static constexpr TagInfo license     = "license,skipmissing";

    static constexpr tags_type tags() {
        return std::tie(name, version, edition, authors, description, license);
    }
};

template <>
struct cpx::Reflect<Carton> //
    : Fields<Reflect<Carton>, &Carton::package, &Carton::features, &Carton::no_default_features> {

    static constexpr TagInfo package             = "package";
    static constexpr TagInfo features            = "features,skipmissing";
    static constexpr TagInfo no_default_features = "no-default-features,skipmissing";

    static constexpr tags_type tags() {
        return std::tie(package, features, no_default_features);
    }
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
