#include <string>
#include <iostream>

import cpx;
import cpx.rapid_json;

struct Address {
    std::string city;
    int         zip;
};

template <>
struct cpx::Reflect<Address> : Fields<Reflect<Address>, &Address::city, &Address::zip> {
    static constexpr cpx::TagInfo city = "city";
    static constexpr cpx::TagInfo zip  = "zip";

    static constexpr tags_type tags() {
        return std::tie(city, zip);
    }
};

int main() {
    Address addr;
    cpx::tuple_for_each(cpx::reflect_of(addr), [](const auto &tagged, size_t) {
        const cpx::TagInfo &tag = tagged.ti;
        std::cout << tag.key << " = " << tagged.value << std::endl;
    });

    std::cout << cpx::rapid_json::io << addr << std::endl;
    return 0;
}
