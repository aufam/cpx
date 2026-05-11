#include <benchmark/benchmark.h>
#include <cpx/proto/protobuf.h>
#include <cpx/fmt.h>
#include "bench/person.pb.h"

using namespace cpx;

namespace {
    struct Person {
        Tag<std::string>      name  = "protobuf:`1`";
        Tag<int>              id    = "protobuf:`2`";
        Tag<std::string>      email = "protobuf:`3`";
        Tag<int>              num1  = "protobuf:`4,fixed`";
        Tag<int>              num2  = "protobuf:`5,zigzag`";
        Tag<int>              num3  = "protobuf:`6`";
        Tag<std::vector<int>> nums  = "protobuf:`7`";
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
} // namespace

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

using Person1 = Person;         // tag
using Person2 = person::Person; // compiled
using Person3 = Person3;        // reflection

static const char data[] = "\x0a\x07Sucipto"
                           "\x10\x2a"
                           "\x1a\x1dsucipto@makmursejahtera.co.id"
                           "\x25\x01\x00\x00\x00"
                           "\x28\x01"
                           ""
                           "\x3a\x03\x00\x00\x00";

static void cpx_tag_protobuf_serialization(benchmark::State &state) {
    std::string buf = std::string(data, sizeof(data) - 1);
    Person1     p;
    cpx::proto::protobuf::parse(buf, p);
    for (auto _ : state) {
        std::string s = cpx::proto::protobuf::dump(p);
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(cpx_tag_protobuf_serialization);

static void cpx_reflect_protobuf_serialization(benchmark::State &state) {
    std::string buf = std::string(data, sizeof(data) - 1);
    Person3     p;
    cpx::proto::protobuf::parse(buf, p);
    for (auto _ : state) {
        std::string s = cpx::proto::protobuf::dump(p);
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(cpx_reflect_protobuf_serialization);

static void compiled_protobuf_serialization(benchmark::State &state) {
    std::string buf = std::string(data, sizeof(data) - 1);
    Person2     p;
    if (!p.ParseFromString(buf))
        throw std::runtime_error("protobuf failed to parse");
    for (auto _ : state) {
        std::string s = p.SerializeAsString();
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(compiled_protobuf_serialization);

static void cpx_tag_protobuf_deserialization(benchmark::State &state) {
    std::string buf = std::string(data, sizeof(data) - 1);
    for (auto _ : state) {
        Person1 p;
        proto::protobuf::parse(buf, p);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(cpx_tag_protobuf_deserialization);

static void cpx_reflect_protobuf_deserialization(benchmark::State &state) {
    std::string buf = std::string(data, sizeof(data) - 1);
    for (auto _ : state) {
        Person3 p;
        proto::protobuf::parse(buf, p);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(cpx_reflect_protobuf_deserialization);

static void compiled_protobuf_deserialization(benchmark::State &state) {
    std::string buf = std::string(data, sizeof(data) - 1);
    for (auto _ : state) {
        Person2 p;
        if (!p.ParseFromString(buf))
            throw std::runtime_error("protobuf failed to parse");
    }
}
BENCHMARK(compiled_protobuf_deserialization);
