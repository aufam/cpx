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
struct cpx::proto::protobuf::Message<const Person3> {
    static constexpr bool value = true;

    static constexpr cpx::TagInfo name  = cpx::TagInfoBuilder("name").field_number(1);
    static constexpr cpx::TagInfo id    = cpx::TagInfoBuilder("id").field_number(2);
    static constexpr cpx::TagInfo email = cpx::TagInfoBuilder("email").field_number(3);
    static constexpr cpx::TagInfo num1  = cpx::TagInfoBuilder("num1").field_number(4).fixed();
    static constexpr cpx::TagInfo num2  = cpx::TagInfoBuilder("num2").field_number(5).zigzag();
    static constexpr cpx::TagInfo num3  = cpx::TagInfoBuilder("num3").field_number(6);
    static constexpr cpx::TagInfo nums  = cpx::TagInfoBuilder("nums").field_number(7);

    const Person3 &p;

    Message(const Person3 &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<const decltype(Person3::name) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Person3::id) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Person3::email) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Person3::num1) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Person3::num2) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Person3::num3) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Person3::nums) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(
            std::tie(p.name, name),
            std::tie(p.id, id),
            std::tie(p.email, email),
            std::tie(p.num1, num1),
            std::tie(p.num2, num2),
            std::tie(p.num3, num3),
            std::tie(p.nums, nums)
        );
    }
};

template <>
struct cpx::proto::protobuf::Message<Person3> {
    static constexpr bool value = true;

    static constexpr cpx::TagInfo name  = cpx::TagInfoBuilder("name").field_number(1);
    static constexpr cpx::TagInfo id    = cpx::TagInfoBuilder("id").field_number(2);
    static constexpr cpx::TagInfo email = cpx::TagInfoBuilder("email").field_number(3);
    static constexpr cpx::TagInfo num1  = cpx::TagInfoBuilder("num1").field_number(4).fixed();
    static constexpr cpx::TagInfo num2  = cpx::TagInfoBuilder("num2").field_number(5).zigzag();
    static constexpr cpx::TagInfo num3  = cpx::TagInfoBuilder("num3").field_number(6);
    static constexpr cpx::TagInfo nums  = cpx::TagInfoBuilder("nums").field_number(7);

    Person3 &p;

    Message(Person3 &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<decltype(Person3::name) &, const cpx::TagInfo &>,
        std::tuple<decltype(Person3::id) &, const cpx::TagInfo &>,
        std::tuple<decltype(Person3::email) &, const cpx::TagInfo &>,
        std::tuple<decltype(Person3::num1) &, const cpx::TagInfo &>,
        std::tuple<decltype(Person3::num2) &, const cpx::TagInfo &>,
        std::tuple<decltype(Person3::num3) &, const cpx::TagInfo &>,
        std::tuple<decltype(Person3::nums) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(
            std::tie(p.name, name),
            std::tie(p.id, id),
            std::tie(p.email, email),
            std::tie(p.num1, num1),
            std::tie(p.num2, num2),
            std::tie(p.num3, num3),
            std::tie(p.nums, nums)
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

    auto pp = cpx::proto::protobuf::parse<Person>(buf);
    fmt::println("person = {}", pp);

    auto e = cpx::proto::protobuf::parse<Person>("");
    fmt::println("empty = {}", e);
}
