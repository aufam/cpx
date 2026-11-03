#include <cpx/toml/marzer_toml.h>
#include <gtest/gtest.h>

using namespace cpx;

namespace {
    struct Person {
        // required fields
        std::string name;
        int         age;

        // optional field
        std::optional<std::string> address;

        // fallback if missing
        std::string department = "unset";

        // omit when empty (go-style omitempty)
        int salary;

        // RFC3339 UTC only: YYYY-MM-DDTHH:MM:SSZ
        std::tm created_at;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Person::name>       = "name",
            cpx::field<&Person::age>        = "age",
            cpx::field<&Person::address>    = "address",
            cpx::field<&Person::department> = "department,skipmissing",
            cpx::field<&Person::salary>     = "salary,omitempty",
            cpx::field<&Person::created_at> = "createdAt",
        };

        // ignored field
        int dummy = 42;

        void *dummy2 = nullptr;
    };

    constexpr const char *toml_full = R"toml(
    name = "Sucipto"
    age = 24
    address = "Jakarta"
    department = "Engineering"
    salary = 1000
    createdAt = 2024-01-02T03:04:05Z
    )toml";

    constexpr const char *toml_missing_department = R"toml(
    name = "Sucipto"
    age = 24
    address = "Jakarta"
    salary = 1000
    createdAt = 2024-01-02T03:04:05Z
    )toml";
} // namespace

TEST(toml, marzer_toml_parse_full) {
    Person p = ::cpx::toml::marzer_toml::parse<Person>(toml_full);

    EXPECT_EQ(p.name, "Sucipto");
    EXPECT_EQ(p.age, 24);

    ASSERT_TRUE(p.address.has_value());

    EXPECT_EQ(*p.address, "Jakarta");
    EXPECT_EQ(p.department, "Engineering");
    EXPECT_EQ(p.salary, 1000);

    const std::tm &t = p.created_at;

    EXPECT_EQ(t.tm_year + 1900, 2024);
    EXPECT_EQ(t.tm_mon + 1, 1);
    EXPECT_EQ(t.tm_mday, 2);

    EXPECT_EQ(t.tm_hour, 3);
    EXPECT_EQ(t.tm_min, 4);
    EXPECT_EQ(t.tm_sec, 5);
}


TEST(toml, marzer_toml_parse_missing_department) {
    Person p = ::cpx::toml::marzer_toml::parse<Person>(toml_missing_department);

    ASSERT_TRUE(p.address.has_value());
    EXPECT_EQ(*p.address, "Jakarta");

    // fallback value
    EXPECT_EQ(p.department, "unset");
}
