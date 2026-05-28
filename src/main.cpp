#include <string>
#include <iostream>
#include <ctime>

import cpx;

struct Address {
    std::string city;
    int         zip;
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

int main(int argc, char **argv) {
    User user;
    std::cout << cpx::has_reflect_v<User> << std::endl;
    return 1;
}
