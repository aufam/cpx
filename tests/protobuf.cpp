#include <cpx/protobuf.h>
#include <gtest/gtest.h>

namespace {
    struct Person3 {
        std::string      name;
        int              id;
        std::string      email;
        int              num1;
        int              num2;
        int              num3;
        std::vector<int> nums;
    };
} // namespace

template <>
struct cpx::Reflect<Person3> {
    [[maybe_unused]]
    static constexpr auto field_tags = std::make_tuple(
        field<&Person3::name>  = "name,field_number=1",
        field<&Person3::id>    = "id,field_number=2",
        field<&Person3::email> = "email,field_number=3",
        field<&Person3::num1>  = "num1,field_number=4,fixed",
        field<&Person3::num2>  = "num2,field_number=5,zigzag",
        field<&Person3::num3>  = "num3,field_number=6,omitempty",
        field<&Person3::nums>  = "7"
    );
};

TEST(protobuf, protobuf) {
    static const char data[] = "\x0a\x07Sucipto"
                               "\x10\x2a"
                               "\x1a\x1dsucipto@makmursejahtera.co.id"
                               "\x25\x01\x00\x00\x00"
                               "\x28\x01"
                               ""
                               "\x3a\x03\x00\x00\x00";

    static_assert(std::get<6>(cpx::Reflect<Person3>::field_tags).tag.field_number == 7);
    static_assert(cpx::protobuf::is_serializable_v<Person3>);

    auto buf = std::string_view(data, sizeof(data) - 1);

    Person3 p;
    {
        cpx::protobuf::parse(buf, p);

        EXPECT_EQ(p.name, "Sucipto");
        EXPECT_EQ(p.id, 42);
        EXPECT_EQ(p.email, "sucipto@makmursejahtera.co.id");
        EXPECT_EQ(p.num1, 1);
        EXPECT_EQ(p.num2, -1);
        EXPECT_EQ(p.num3, 0);
        EXPECT_EQ(p.nums, (std::vector<int>{0, 0, 0}));
    }

    std::string s = cpx::protobuf::dump(p);
    EXPECT_EQ(s, buf);
}
