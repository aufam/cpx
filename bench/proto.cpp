#include <benchmark/benchmark.h>
#include <cpx/protobuf.h>
#include <cpx/msgpack.h>
#include "bench/person.pb.h"

using namespace cpx;

namespace {
    struct Person {
        Tag<std::string>      name  = "protobuf,msgpack:`1`";
        Tag<int>              id    = "protobuf,msgpack:`2`";
        Tag<std::string>      email = "protobuf,msgpack:`3`";
        Tag<int>              num1  = "protobuf,msgpack:`4,fixed`";
        Tag<int>              num2  = "protobuf,msgpack:`5,zigzag`";
        Tag<int>              num3  = "protobuf,msgpack:`6`";
        Tag<std::vector<int>> nums  = "protobuf,msgpack:`7`";
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
struct cpx::Reflect<Person3> {
    [[maybe_unused]]
    static constexpr auto field_tags = std::make_tuple(
        field<&Person3::name>  = "name,field_number=1",
        field<&Person3::id>    = "id,field_number=2",
        field<&Person3::email> = "email,field_number=3",
        field<&Person3::num1>  = "num1,field_number=4,fixed",
        field<&Person3::num2>  = "num2,field_number=5,zigzag",
        field<&Person3::num3>  = "num3,field_number=6",
        field<&Person3::nums>  = "7"
    );
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

const char msgpack_data[] = "\x87"
                            "\x01\xA7Sucipto"
                            "\x02\x2a"
                            "\x03\xBDsucipto@makmursejahtera.co.id"
                            "\x04\x01"
                            "\x05\x01"
                            "\x06\x00"
                            "\x07\x93\x00\x00\x00";

static void cpx_tag_protobuf_serialization(benchmark::State &state) {
    std::string buf = std::string(data, sizeof(data) - 1);
    Person1     p;
    cpx::protobuf::parse(buf, p);
    for (auto _ : state) {
        std::string s = cpx::protobuf::dump(p);
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK(cpx_tag_protobuf_serialization);

static void cpx_reflect_protobuf_serialization(benchmark::State &state) {
    std::string buf = std::string(data, sizeof(data) - 1);
    Person3     p;
    cpx::protobuf::parse(buf, p);
    for (auto _ : state) {
        std::string s = cpx::protobuf::dump(p);
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK(cpx_reflect_protobuf_serialization);

static void cpx_tag_msgpack_serialization(benchmark::State &state) {
    std::string buf = std::string(msgpack_data, sizeof(msgpack_data) - 1);
    Person1     p;
    cpx::msgpack::parse(buf, p);
    for (auto _ : state) {
        std::string s = cpx::msgpack::dump(p);
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK(cpx_tag_msgpack_serialization);

static void cpx_reflect_msgpack_serialization(benchmark::State &state) {
    std::string buf = std::string(msgpack_data, sizeof(msgpack_data) - 1);
    Person3     p;
    cpx::msgpack::parse(buf, p);
    for (auto _ : state) {
        std::string s = cpx::msgpack::dump(p);
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK(cpx_reflect_msgpack_serialization);

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
        cpx::protobuf::parse(buf, p);
        benchmark::DoNotOptimize(p);
    }
}

BENCHMARK(cpx_tag_protobuf_deserialization);

static void cpx_reflect_protobuf_deserialization(benchmark::State &state) {
    std::string buf = std::string(data, sizeof(data) - 1);
    for (auto _ : state) {
        Person3 p;
        cpx::protobuf::parse(buf, p);
        benchmark::DoNotOptimize(p);
    }
}

BENCHMARK(cpx_reflect_protobuf_deserialization);

static void cpx_tag_msgpack_deserialization(benchmark::State &state) {
    std::string buf = std::string(msgpack_data, sizeof(msgpack_data) - 1);
    for (auto _ : state) {
        Person1 p;
        cpx::msgpack::parse(buf, p);
        benchmark::DoNotOptimize(p);
    }
}

BENCHMARK(cpx_tag_msgpack_deserialization);

static void cpx_reflect_msgpack_deserialization(benchmark::State &state) {
    std::string buf = std::string(msgpack_data, sizeof(msgpack_data) - 1);
    for (auto _ : state) {
        Person3 p;
        cpx::msgpack::parse(buf, p);
        benchmark::DoNotOptimize(p);
    }
}

BENCHMARK(cpx_reflect_msgpack_deserialization);

static void compiled_protobuf_deserialization(benchmark::State &state) {
    std::string buf = std::string(data, sizeof(data) - 1);
    for (auto _ : state) {
        Person2 p;
        if (!p.ParseFromString(buf))
            throw std::runtime_error("protobuf failed to parse");
    }
}

BENCHMARK(compiled_protobuf_deserialization);
