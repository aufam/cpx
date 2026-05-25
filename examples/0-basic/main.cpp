// single include libs
// #include "maybe/you/place/nlohmann/json/in/a/weird/path/single_include/json.h"
// #include "as/well/as/toml++.h"

#include <cpx/fmt.h>                // includes fmt/ranges.h
#include <cpx/toml/marzer_toml.h>   // does not include toml++ if already included
#include <cpx/json/nlohmann_json.h> // does not include nlohmann/json.h if already included
#include <cpx/json/rapid_json.h>    // includes RapidJSON reader/writer headers
#include <cpx/proto/protobuf.h>     // includes protobuf coded stream headers
#include <iostream>

int main() {
    constexpr cpx::TagInfo name_tag     = "name,field_number=1";
    constexpr cpx::TagInfo nickname_tag = "nickname"; // protobuf will skip this
    constexpr cpx::TagInfo age_tag      = "age,field_number=2";
    constexpr cpx::TagInfo address_tag  = "address,field_number=3";
    constexpr cpx::TagInfo city_tag     = "city,field_number=1";
    constexpr cpx::TagInfo zip_tag      = "zip,field_number=2"
        // ",omitempty" // uncomment this line to see the output diff
        ;

    std::string        name     = "Sucipto";
    std::string        nickname = "The Shark";
    int                age      = 23;
    std::string        city     = "Tanjung Priok";
    std::optional<int> zip_code = std::nullopt; // if not omitempty, toml will output 0

    std::tuple address = {
        cpx::tag_tie(city, city_tag),
        cpx::tag_tie(zip_code, zip_tag),
    };

    std::tuple p = {
        cpx::tag_tie(name, name_tag),
        cpx::tag_tie(nickname, nickname_tag),
        cpx::tag_tie(age, age_tag),
        cpx::tag_tie(address, address_tag),
    };

    // formatting using fmtlib
    fmt::println("fmt = {}", p);

    // dump interface
    fmt::println("toml++ = {:?}", cpx::toml::marzer_toml::dump(p));
    fmt::println("protobuf = {:?}", cpx::proto::protobuf::dump(p));

    // native adl serializer specialization
    nlohmann::json j = p;
    fmt::println("nlohmann_json = {}", j.dump());
    // the same with
    // fmt::println("nlohmann_json = {}", cpx::json::nlohmann_json::dump(p));

    // SAX streaming (if lib supports)
    std::cout << "rapidjson = " << cpx::json::rapid_json::io << p << std::endl;
}
