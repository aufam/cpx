#include <cpx/yaml/jbeder_yaml.h>
#include <gtest/gtest.h>

using namespace cpx;

namespace {
    struct Person {
        // required fields
        Tag<std::string> name = "yaml:`name`";
        Tag<int>         age  = "yaml:`age`";

        // optional field
        Tag<std::optional<std::string>> address = "yaml:`address`";

        // fallback if missing
        Tag<std::string> department = {"yaml:`department,skipmissing`", "unset"};

        // omit when empty (go-style omitempty)
        Tag<int> salary = "yaml:`salary,omitempty`";

        // RFC3339 UTC only: YYYY-MM-DDTHH:MM:SSZ
        Tag<std::tm> created_at = "yaml:`createdAt`";

        // ignored fields
        int   dummy = 42;
        void *ptr   = nullptr;
    };

    constexpr const char *yaml_full = R"yaml(
        name: "Sucipto"
        age: 24
        address: "Jakarta"
        department: "Engineering"
        salary: 1000
        createdAt: 2024-01-02T03:04:05Z
    )yaml";

    constexpr const char *yaml_missing_department = R"yaml(
        name: "Sucipto"
        age: 24
        address: "Jakarta"
        salary: 1000
        createdAt: 2024-01-02T03:04:05Z
    )yaml";
} // namespace

TEST(yaml, jbeder_yaml_parse_full) {
    Person p = ::cpx::yaml::jbeder_yaml::parse<Person>(yaml_full);

    EXPECT_EQ(p.name(), "Sucipto");
    EXPECT_EQ(p.age(), 24);

    ASSERT_TRUE(p.address().has_value());

    EXPECT_EQ(*p.address(), "Jakarta");
    EXPECT_EQ(p.department(), "Engineering");
    EXPECT_EQ(p.salary(), 1000);

    const std::tm &t = p.created_at();

    EXPECT_EQ(t.tm_year + 1900, 2024);
    EXPECT_EQ(t.tm_mon + 1, 1);
    EXPECT_EQ(t.tm_mday, 2);

    EXPECT_EQ(t.tm_hour, 3);
    EXPECT_EQ(t.tm_min, 4);
    EXPECT_EQ(t.tm_sec, 5);
}


TEST(yaml, jbeder_yaml_parse_missing_department) {
    Person p = ::cpx::yaml::jbeder_yaml::parse<Person>(yaml_missing_department);

    ASSERT_TRUE(p.address().has_value());
    EXPECT_EQ(*p.address(), "Jakarta");

    // fallback value
    EXPECT_EQ(p.department(), "unset");
}
