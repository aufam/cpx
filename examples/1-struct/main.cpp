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
    std::tm     joined;
    Address     address;
};

constexpr cpx::TagInfo name_tag    = "name";
constexpr cpx::TagInfo age_tag     = "age";
constexpr cpx::TagInfo joined_tag  = "joined";
constexpr cpx::TagInfo address_tag = "address";
constexpr cpx::TagInfo city_tag    = "city";
constexpr cpx::TagInfo zip_tag     = "zip";

template <>
struct cpx::Reflect<User>                    //
    : Fields<                                //
          Field<&User::name, name_tag>,      //
          Field<&User::age, age_tag>,        //
          Field<&User::joined, joined_tag>,  //
          Field<&User::address, address_tag> //
          > {};

template <>
struct cpx::Reflect<Address>               //
    : Fields<                              //
          Field<&Address::city, city_tag>, //
          Field<&Address::zip, zip_tag>    //
          > {};


int main() {
    User sucipto;

    const char *jdoc = R"json({
        "name": "Sucipto",
        "age": 42,
        "joined": "2023-10-27T10:00:00Z",
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
        joined = 2023-10-27T10:30:00Z

        [address]
        city = "Hamburg"
        zip = 9021
    )toml";

    cpx::toml::marzer_toml::parse(tdoc, marwoto);
    fmt::println("data = {}", marwoto);
    fmt::println("toml = {:?}", cpx::toml::marzer_toml::dump(marwoto));
}
