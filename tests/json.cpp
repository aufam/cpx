#include <cpx/json/yy_json.h>
#include <cpx/json/nlohmann_json.h>
#include <gtest/gtest.h>


using namespace cpx;

namespace {

    struct Person {
        // required fields
        Tag<std::string> name = "json:`name`";
        Tag<int>         age  = "json:`age`";

        // optional field
        Tag<std::optional<std::string>> address = "json:`address`";

        // fallback if missing
        Tag<std::string> department = {"json:`department,skipmissing`", "unset"};

        // omit when empty (go-style omitempty)
        Tag<int> salary = "json:`salary,omitempty`";

        // RFC3339 UTC only: YYYY-MM-DDTHH:MM:SSZ
        Tag<std::tm> created_at = "json:`createdAt`";

        // ignored field
        int dummy = 42;

        void *dummy2 = nullptr;
    };

    static_assert(std::is_aggregate_v<Person>, "Person must be pure aggregate");

    static_assert(serde::is_serializable<yyjson_mut_val, Person>::value);
    static_assert(serde::is_deserializable<yyjson_val, Person>::value);

    static_assert(!serde::is_serializable<yyjson_mut_val, void *>::value);
    static_assert(!serde::is_deserializable<yyjson_val, void *>::value);

    static_assert(serde::is_serializable<nlohmann::json, Person>::value);
    static_assert(serde::is_deserializable<nlohmann::json, Person>::value);

    static_assert(!serde::is_serializable<nlohmann::json, void *>::value);
    static_assert(!serde::is_deserializable<nlohmann::json, void *>::value);

    static_assert(serde::is_serializable<nlohmann::json, std::unordered_map<std::string, std::string>>::value);

    constexpr const char *json_full = R"json(
        {
          "name": "Sucipto",
          "age": 24,
          "address": "Jakarta",
          "department": "Engineering",
          "salary": 1000,
          "createdAt": "2024-01-02T03:04:05Z"
        }
    )json";


    constexpr const char *json_missing_department = R"json(
        {
          "name": "Sucipto",
          "age": 24,
          "address": "Jakarta",
          "salary": 1000,
          "createdAt": "2024-01-02T03:04:05Z"
        }
    )json";

} // namespace


namespace {
    struct Person2 {
        std::string                name;
        int                        age;
        std::optional<std::string> address;
        std::string                department = "unset";
        int                        salary     = 0;
        std::tm                    created_at = {};
        int                        dummy      = 42;
        void                      *dummy2     = nullptr;
    };
} // namespace


template <typename T>
struct member_pointer_traits;

template <typename Class, typename Member>
struct member_pointer_traits<Member Class::*> {
    using class_type  = Class;
    using member_type = Member;
};

template <auto MemberPtr>
struct field {
    using traits = member_pointer_traits<decltype(MemberPtr)>;

    using class_type  = typename traits::class_type;
    using member_type = typename traits::member_type;

    TagInfo tag;

    constexpr explicit field(TagInfo tag)
        : tag(tag) {}

    constexpr decltype(auto) get(class_type &obj) const {
        return obj.*MemberPtr;
    }

    constexpr decltype(auto) get(const class_type &obj) const {
        return obj.*MemberPtr;
    }
};

template <>
struct cpx::serde::SerializeAs<cpx::json::JsonGeneric, Person2> : std::true_type {
private:
    static constexpr cpx::TagInfo name       = cpx::TagInfoBuilder("name");
    static constexpr cpx::TagInfo age        = cpx::TagInfoBuilder("age");
    static constexpr cpx::TagInfo address    = cpx::TagInfoBuilder("address");
    static constexpr cpx::TagInfo department = cpx::TagInfoBuilder("department").omitempty();
    static constexpr cpx::TagInfo salary     = cpx::TagInfoBuilder("salary").omitempty();
    static constexpr cpx::TagInfo created_at = cpx::TagInfoBuilder("createdAt");

    const Person2 &p;

public:
    SerializeAs(const Person2 &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<const decltype(Person2::name) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Person2::age) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Person2::address) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Person2::department) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Person2::salary) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Person2::created_at) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(
            std::tie(p.name, name),
            std::tie(p.age, age),
            std::tie(p.address, address),
            std::tie(p.department, department),
            std::tie(p.salary, salary),
            std::tie(p.created_at, created_at)
        );
    }
};

template <>
struct cpx::serde::DeserializeAs<cpx::json::JsonGeneric, Person2> : std::true_type {
private:
    static constexpr cpx::TagInfo name       = cpx::TagInfoBuilder("name");
    static constexpr cpx::TagInfo age        = cpx::TagInfoBuilder("age");
    static constexpr cpx::TagInfo address    = cpx::TagInfoBuilder("address");
    static constexpr cpx::TagInfo department = cpx::TagInfoBuilder("department").omitempty();
    static constexpr cpx::TagInfo salary     = cpx::TagInfoBuilder("salary").omitempty();
    static constexpr cpx::TagInfo created_at = cpx::TagInfoBuilder("createdAt");

    Person2 &p;

public:
    DeserializeAs(Person2 &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<decltype(Person2::name) &, const cpx::TagInfo &>,
        std::tuple<decltype(Person2::age) &, const cpx::TagInfo &>,
        std::tuple<decltype(Person2::address) &, const cpx::TagInfo &>,
        std::tuple<decltype(Person2::department) &, const cpx::TagInfo &>,
        std::tuple<decltype(Person2::salary) &, const cpx::TagInfo &>,
        std::tuple<decltype(Person2::created_at) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(
            std::tie(p.name, name),
            std::tie(p.age, age),
            std::tie(p.address, address),
            std::tie(p.department, department),
            std::tie(p.salary, salary),
            std::tie(p.created_at, created_at)
        );
    }
};

static_assert(serde::is_serializable<yyjson_mut_val, Person2>::value);

TEST(json, yy_json_parse_full) {
    Person p = json::yy_json::parse<Person>(json_full);

    EXPECT_EQ(p.name(), "Sucipto");
    EXPECT_EQ(p.age(), 24);

    ASSERT_TRUE(p.address().has_value());

    EXPECT_EQ(*p.address(), "Jakarta");
    EXPECT_EQ(p.department(), "Engineering");
    EXPECT_EQ(p.salary(), 1000);

    const std::tm &t = p.created_at();

    EXPECT_EQ(t.tm_year + 1900, 2024);
    EXPECT_EQ(t.tm_mon + 1, 1);
    EXPECT_EQ(t.tm_mday, 2);

    EXPECT_EQ(t.tm_hour, 3);
    EXPECT_EQ(t.tm_min, 4);
    EXPECT_EQ(t.tm_sec, 5);
}


TEST(json, yy_json_parse_missing_department) {
    Person p = json::yy_json::parse<Person>(json_missing_department);

    ASSERT_TRUE(p.address().has_value());
    EXPECT_EQ(*p.address(), "Jakarta");

    // fallback value
    EXPECT_EQ(p.department(), "unset");
}


TEST(json, yy_json_dump_omitempty) {
    Person2 p;
    p.name = "Sucipto";
    p.age  = 24;

    std::string dumped = json::yy_json::dump(p);

    EXPECT_NE(dumped.find("\"name\":\"Sucipto\""), std::string::npos);
    EXPECT_NE(dumped.find("\"age\":24"), std::string::npos);

    EXPECT_NE(dumped.find("\"address\":null"), std::string::npos);
    EXPECT_NE(dumped.find("\"department\":\"unset\""), std::string::npos);

    // omitted fields
    EXPECT_EQ(dumped.find("salary"), std::string::npos);
}


TEST(cpx, nlohmann_json_parse_full) {
    Person p = nlohmann::json::parse(json_full);

    EXPECT_EQ(p.name(), "Sucipto");
    EXPECT_EQ(p.age(), 24);

    ASSERT_TRUE(p.address().has_value());
    EXPECT_EQ(*p.address(), "Jakarta");

    EXPECT_EQ(p.department(), "Engineering");
    EXPECT_EQ(p.salary(), 1000);
}


TEST(json, nlohmann_json_dump_omitempty) {
    Person p;
    p.name() = "Sucipto";
    p.age()  = 24;

    std::string dumped = nlohmann::json(p).dump();

    EXPECT_NE(dumped.find("\"name\":\"Sucipto\""), std::string::npos);
    EXPECT_NE(dumped.find("\"age\":24"), std::string::npos);

    EXPECT_NE(dumped.find("\"address\":null"), std::string::npos);
    EXPECT_NE(dumped.find("\"department\":\"unset\""), std::string::npos);

    EXPECT_EQ(dumped.find("salary"), std::string::npos);
}


TEST(json, yy_json_round_trip) {
    Person      p1     = json::yy_json::parse<Person>(json_full);
    std::string dumped = json::yy_json::dump(p1);
    Person      p2     = json::yy_json::parse<Person>(dumped);

    EXPECT_EQ(p2.name(), p1.name());
    EXPECT_EQ(p2.age(), p1.age());
    EXPECT_EQ(p2.department(), p1.department());
}
