#include <cpx/fmt.h>
#include <cpx/json/yy_json.h>
#include <cpx/toml/marzer_toml.h>

struct Address {
    std::string        city;
    std::optional<int> zip;

    static constexpr cpx::TagInfo _city = "city";
    static constexpr cpx::TagInfo _zip  = "zip";
};

struct User {
    std::string name;
    int         age;
    std::tm     created_at;
    Address     address;

    static constexpr cpx::TagInfo _name            = "name";
    static constexpr cpx::TagInfo _age             = "age";
    static constexpr cpx::TagInfo _created_at      = "created-at";
    static constexpr cpx::TagInfo _created_at_json = "createdAt";
    static constexpr cpx::TagInfo _address         = "address";
};

template <>
struct cpx::Reflect<User>                              //
    : Fields<                                          //
          Field<&User::name, User::_name>,             //
          Field<&User::age, User::_age>,               //
          Field<&User::created_at, User::_created_at>, //
          Field<&User::address, User::_address>        //
          > {};

template <>
struct cpx::json::Reflect<User>                             //
    : Fields<                                               //
          Field<&User::name, User::_name>,                  //
          Field<&User::age, User::_age>,                    //
          Field<&User::created_at, User::_created_at_json>, //
          Field<&User::address, User::_address>             //
          > {};

template <>
struct cpx::Reflect<Address>                     //
    : Fields<                                    //
          Field<&Address::city, Address::_city>, //
          Field<&Address::zip, Address::_zip>    //
          > {};


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
