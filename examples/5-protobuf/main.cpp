#include <cpx/proto/protobuf.h>
#include <cpx/fmt.h>
#include <cpx/reflect.h>

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
struct cpx::Reflect<Person3>
    : cpx::Fields<&Person3::name, &Person3::id, &Person3::email, &Person3::num1, &Person3::num2, &Person3::num3, &Person3::nums> {
    static constexpr cpx::TagInfo name  = "name,field_number=1";
    static constexpr cpx::TagInfo id    = "id,field_number=2";
    static constexpr cpx::TagInfo email = "email,field_number=3";
    static constexpr cpx::TagInfo num1  = "num1,field_number=4";
    static constexpr cpx::TagInfo num2  = "num2,field_number=5";
    static constexpr cpx::TagInfo num3  = "num3,field_number=6";
    static constexpr cpx::TagInfo nums  = "7";

    static const_type of(const Person3 &p) {
        return std::make_tuple(
            cpx::tag_tie(p.name, name),
            cpx::tag_tie(p.id, id),
            cpx::tag_tie(p.email, email),
            cpx::tag_tie(p.num1, num1),
            cpx::tag_tie(p.num2, num2),
            cpx::tag_tie(p.num3, num3),
            cpx::tag_tie(p.nums, nums)
        );
    }

    static type of(Person3 &p) {
        return std::make_tuple(
            cpx::tag_tie(p.name, name),
            cpx::tag_tie(p.id, id),
            cpx::tag_tie(p.email, email),
            cpx::tag_tie(p.num1, num1),
            cpx::tag_tie(p.num2, num2),
            cpx::tag_tie(p.num3, num3),
            cpx::tag_tie(p.nums, nums)
        );
    }
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
        buf = cpx::proto::protobuf::dump(p);
        fmt::println("proto = {:02x}", fmt::join(buf, " "));
    }
    {
        std::string buf = cpx::proto::protobuf::dump(std::tie(p.name));
        fmt::println("p.name = {:02x}", fmt::join(buf, " "));
    }
    {
        std::string buf = cpx::proto::protobuf::dump(std::tie(p.id));
        fmt::println("p.id = {:02x}", fmt::join(buf, " "));
    }
    {
        std::string buf = cpx::proto::protobuf::dump(std::tie(p.email));
        fmt::println("p.email = {:02x}", fmt::join(buf, " "));
    }
    {
        std::string buf = cpx::proto::protobuf::dump(std::tie(p.num1));
        fmt::println("p.num1 = {:02x}", fmt::join(buf, " "));
    }
    {
        std::string buf = cpx::proto::protobuf::dump(std::tie(p.num2));
        fmt::println("p.num2 = {:02x}", fmt::join(buf, " "));
    }
    {
        std::string buf = cpx::proto::protobuf::dump(std::tie(p.num3));
        fmt::println("p.num3 = {:02x}", fmt::join(buf, " "));
    }
    {
        std::string buf = cpx::proto::protobuf::dump(std::tie(p.nums));
        fmt::println("p.num3 = {:02x}", fmt::join(buf, " "));
    }

    auto pp = cpx::proto::protobuf::parse<Person3>(buf);
    fmt::println("person = {}", pp);

    auto e = cpx::proto::protobuf::parse<Person>("");
    fmt::println("empty = {}", e);
}
