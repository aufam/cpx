#include <string>
#include <iostream>

import cpx;
import cpx.rapid_json;

struct Address {
    std::string city;
    int         zip;

    static constexpr std::tuple __field_tags__ = {
        cpx::field<&Address::city> = "city",
        cpx::field<&Address::zip>  = "zip",
    };
};

int main() {
    Address addr;
    cpx::tuple_for_each(cpx::reflect_traits<Address>::of(addr), [](const auto &tagged, size_t) {
        const cpx::TagInfo &tag = tagged.ti;
        std::cout << tag.key << " = " << tagged.value << std::endl;
    });

    std::cout << cpx::rapid_json::io << addr << std::endl;
    return 0;
}
