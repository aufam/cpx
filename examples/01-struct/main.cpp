#include <cpx/fmt.h>
#include <cpx/json/yy_json.h>
#include <cpx/toml/marzer_toml.h>

struct Address {
    std::string        city;
    std::optional<int> zip;

    static constexpr auto __field_tags__ = std::make_tuple(
        cpx::field<&Address::city> = "city", //
        cpx::field<&Address::zip>  = "zip"
    );
};

struct User {
    std::string name;
    int         age;
    std::tm     created_at;
    Address     address;
};

template <>
struct cpx::Reflect<User> {
    [[maybe_unused]]
    static constexpr auto field_tags = std::make_tuple(
        cpx::field<&User::name>       = "name",
        cpx::field<&User::age>        = "age",
        cpx::field<&User::created_at> = "created-at",
        cpx::field<&User::address>    = "address"
    );
};

template <>
struct cpx::json::Reflect<User> {
    [[maybe_unused]]
    static constexpr auto field_tags = std::make_tuple(
        cpx::field<&User::name>       = "name",
        cpx::field<&User::age>        = "age",
        cpx::field<&User::created_at> = "createdAt",
        cpx::field<&User::address>    = "address"
    );
};

int main() {
    User sucipto;

    const char *jdoc = R"json({
        "name": "Sucipto",
        "age": 42,
        "createdAt": "2023-10-27T10:00:00Z",
        "address": {
            "city": "Oslo"
        }
    })json";

    cpx::json::yy_json::parse(jdoc, sucipto);
    fmt::println("data = {}", sucipto);
    fmt::println("json = {}", cpx::json::yy_json::dump(sucipto));


    User marwoto;

    const char *tdoc = R"toml(
        name = "Marwoto"
        age = 32
        created-at = 2023-10-27T10:30:00Z

        [address]
        city = "Hamburg"
        zip = 9021
    )toml";

    cpx::toml::marzer_toml::parse(tdoc, marwoto);
    fmt::println("data = {}", marwoto);
    fmt::println("toml = {:?}", cpx::toml::marzer_toml::dump(marwoto));
}
