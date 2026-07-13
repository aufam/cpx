#include <cpx/protobuf.h>
#include <cpx/fmt.h>
#include <fstream>


template <typename T>
using Tag = cpx::Tag<T>;

struct Person {
    Tag<std::string> name  = "fmt:`name`    protobuf:`1`";
    Tag<int>         id    = "fmt:`id`      protobuf:`2`";
    Tag<std::string> email = "fmt:`email`   protobuf:`3`";
    Tag<int>         num1  = "fmt:`num1`    protobuf:`4,fixed`";
    Tag<int>         num2  = "fmt:`num2`    protobuf:`5,zigzag`";
    Tag<int>         num3  = "fmt:`num3`    protobuf:`6`";

    Tag<std::vector<int>> nums = "fmt:`nums`    protobuf:`7`";
};

struct Person3 {
    std::string      name;
    int              id;
    std::string      email;
    int              num1;
    int              num2;
    int              num3;
    std::vector<int> nums;
};

template <>
struct cpx::Reflect<Person3> {
    [[maybe_unused]]
    static constexpr auto field_tags = std::make_tuple(
        field<&Person3::name>  = "name,field_number=1",
        field<&Person3::id>    = "id,field_number=2",
        field<&Person3::email> = "email,field_number=3",
        field<&Person3::num1>  = "num1,field_number=4,fixed",
        field<&Person3::num2>  = "num2,field_number=1,zigzag",
        field<&Person3::num3>  = "num3,field_number=1",
        field<&Person3::nums>  = "7"
    );
};

int main() {
    Person p;
    p.name()  = "Sucipto";
    p.id()    = 42;
    p.email() = "sucipto@makmursejahtera.co.id";
    p.num1()  = 1;
    p.num2()  = -1;
    p.num3()  = 0;
    p.nums()  = {0, 0, 0};
    fmt::println("person = {}", p);

    std::string buf;
    {
        buf = cpx::protobuf::dump(p);
        fmt::println("proto = {:02x}", fmt::join(buf, " "));
    }
    {
        std::string buf = cpx::protobuf::dump(std::tie(p.name));
        fmt::println("p.name = {:02x}", fmt::join(buf, " "));
    }
    {
        std::string buf = cpx::protobuf::dump(std::tie(p.id));
        fmt::println("p.id = {:02x}", fmt::join(buf, " "));
    }
    {
        std::string buf = cpx::protobuf::dump(std::tie(p.email));
        fmt::println("p.email = {:02x}", fmt::join(buf, " "));
    }
    {
        std::string buf = cpx::protobuf::dump(std::tie(p.num1));
        fmt::println("p.num1 = {:02x}", fmt::join(buf, " "));
    }
    {
        std::string buf = cpx::protobuf::dump(std::tie(p.num2));
        fmt::println("p.num2 = {:02x}", fmt::join(buf, " "));
    }
    {
        std::string buf = cpx::protobuf::dump(std::tie(p.num3));
        fmt::println("p.num3 = {:02x}", fmt::join(buf, " "));
    }
    {
        std::string buf = cpx::protobuf::dump(std::tie(p.nums));
        fmt::println("p.num3 = {:02x}", fmt::join(buf, " "));
    }

    auto pp = cpx::protobuf::parse<Person3>(buf);
    fmt::println("person = {}", pp);

    auto e = cpx::protobuf::parse<Person3>("");
    fmt::println("empty = {}", e);

    {
        std::ofstream ofs("data.bin", std::ios::binary);
        ofs << cpx::protobuf::io << pp;
    }

    {
        Person3       p;
        std::ifstream ifs("data.bin", std::ios::binary);
        ifs >> cpx::protobuf::io >> p;
        fmt::println("data.bin = {}", p);
    }
}
