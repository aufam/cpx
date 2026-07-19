// single include libs
// #include "maybe/you/place/nlohmann/json/in/a/weird/path/single_include/json.h"
// #include "as/well/as/toml++.h"

#include <cpx/fmt.h>                // includes fmt/ranges.h
#include <cpx/toml/marzer_toml.h>   // does not include toml++ if already included
#include <cpx/json/nlohmann_json.h> // does not include nlohmann/json.h if already included
#include <cpx/json/rapid_json.h>    // includes RapidJSON reader/writer headers
#include <cpx/protobuf.h>           // includes protobuf coded stream headers
#include <iostream>

int main() {
    std::string        name     = "Sucipto";
    std::string        nickname = "The Shark";
    int                age      = 23;
    std::string        city     = "Tanjung Priok";
    std::optional<int> zip_code = std::nullopt; // if not omitempty, toml will output 0

    std::tuple address = {
        cpx::field_ref(city)     = "city,field_number=1",
        cpx::field_ref(zip_code) = "zip,field_number=2",
    };

    std::tuple p = {
        cpx::field_ref(name)     = "name,field_number=1",
        cpx::field_ref(nickname) = "nickname", // protobuf will skip this
        cpx::field_ref(age)      = "age,field_number=2",
        cpx::field_ref(address)  = "address,field_number=3"
        // ",omitempty" // uncomment this line to see the output diff
    };

    // formatting using fmtlib
    fmt::println("fmt = {}", p);

    // dump interface
    fmt::println("toml++ = {:?}", cpx::toml::marzer_toml::dump(p));
    fmt::println("protobuf = {:?}", cpx::protobuf::dump(p));

    // native adl serializer specialization
    nlohmann::json j = p;
    fmt::println("nlohmann_json = {}", j.dump());
    // the same with
    // fmt::println("nlohmann_json = {}", cpx::json::nlohmann_json::dump(p));

    // SAX streaming (if lib supports)
    std::cout << "rapidjson = " << cpx::json::rapid_json::io << p << std::endl;
}
