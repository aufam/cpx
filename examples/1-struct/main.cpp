#include <cpx/fmt.h>
#include <cpx/json/yy_json.h>
#include <cpx/toml/marzer_toml.h>


template <typename T>
using Tag = cpx::Tag<T>;


struct Data {
    struct Inner {
        Tag<std::string> text = "fmt,json,toml:`text`";
        Tag<float>       pi   = {"fmt,json,toml:`pi,skipmissing`", 3.14f};
    };

    Tag<int>                        number = "fmt,json,toml:`number`";
    Tag<std::tm>                    dt     = "fmt:`dt` json:`dateTime` toml:`date-time`";
    Tag<Inner>                      inner  = "fmt,json,toml:`inner`";
    Tag<std::optional<std::string>> text   = "fmt,json,toml:`text,omitempty`";
    int                             dummy;
};

static_assert(std::is_aggregate_v<Data> && std::is_aggregate_v<Data::Inner>);


class Foo {
public:
    Foo(int a)
        : a(a) {}

    int a;
};

struct Bar {
    int a;
};

template <>
struct cpx::Reflect<const Foo> : cpx::Reflect<const Foo, &Foo::a> {
    static constexpr cpx::TagInfo a = cpx::TagInfoBuilder("a");

    using Reflect<const Foo, &Foo::a>::Reflect;

    operator type() const {
        return std::make_tuple(cpx::tag_tie(p.a, a));
    }
};

constexpr bool asfsd  = cpx::serde::is_serializable_v<yyjson_mut_val, Foo>;
constexpr bool asfsd2 = cpx::has_reflect_v<Bar>;
using afsf            = typename cpx::reflect_t<const Foo>;
constexpr bool asfsd3 = fmt::is_formattable<const Foo>::value;

int main() {
    Data data; // .dummy is expected to be undefined

    const char *jdoc = R"json({
        "number": 42,
        "dateTime": "2023-10-27T10:00:00Z",
        "inner": {
            "text": "from json"
        }
    })json";

    cpx::json::yy_json::parse(jdoc, data);
    fmt::println("data = {}", data);
    fmt::println("json = {}", cpx::json::yy_json::dump(data));


    const char *tdoc = R"toml(
        number = 37
        date-time = 2023-10-27T10:30:00Z

        [inner]
        text = "From toml"
        pi = 3.0
    )toml";

    cpx::toml::marzer_toml::parse(tdoc, data);
    fmt::println("data = {}", data);
    fmt::println("=== toml ===\n{}", cpx::toml::marzer_toml::dump(data));

    const Foo foo(10);
    fmt::println("foo = {}", cpx::json::yy_json::dump(foo));
}
