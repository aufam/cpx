#include <cpx/fmt.h>
#include <cpx/json/yy_json.h>
#include <cpx/toml/marzer_toml.h>

struct Address {
    std::string        city;
    std::optional<int> zip;
};

struct User {
    std::string name;
    int         age;
    std::tm     created_at;
    Address     address;
};

template <>
struct cpx::Reflect<User> : FieldsV2<Reflect<User>, &User::name, &User::age, &User::created_at, &User::address> {
    static constexpr TagInfo name = "name", age = "age", created_at = "created-at", address = "address";

    static constexpr tags_type tags() {
        return std::tie(name, age, created_at, address);
    }
};

template <>
struct cpx::json::Reflect<User> : FieldsV2<Reflect<User>, &User::name, &User::age, &User::created_at, &User::address> {
    static constexpr TagInfo created_at = "createdAt";

    static constexpr tags_type tags() {
        using Base = cpx::Reflect<User>;
        return std::tie(Base::name, Base::age, created_at, Base::address);
    }
};

template <>
struct cpx::Reflect<Address> : FieldsV2<Reflect<Address>, &Address::city, &Address::zip> {
    static constexpr std::tuple<TagInfo, TagInfo> _tags = {"city", "zip"};

    static constexpr tags_type tags() {
        return _tags;
    }
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
