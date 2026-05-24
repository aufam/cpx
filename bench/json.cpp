#include <cpx/json/yy_json.h>
#include <cpx/json/rapid_json.h>
#include <cpx/json/nlohmann_json.h>
#include <cpx/reflect.h>
#include <cpx/fields.h>
#include <benchmark/benchmark.h>


template <typename T>
using Tag = cpx::Tag<T>;

namespace {
    struct Schema {
        struct User {
            Tag<int>                      id       = "json:`id`";
            Tag<std::string>              username = "json:`username`";
            Tag<std::string>              email    = "json:`email`";
            Tag<std::vector<std::string>> roles    = "json:`roles`";

            struct Profile {
                Tag<int>         age     = "json:`age`";
                Tag<std::string> country = "json:`country`";

                struct Preferences {
                    Tag<bool> dark_mode = "json:`darkMode`";
                    struct Notifications {
                        Tag<bool> email = "json:`email`";
                        Tag<bool> push  = "json:`push`";
                    };
                    Tag<Notifications> notifications = "json:`notifications`";
                };
                Tag<Preferences> preferences = "json:`preferences`";
            };
            Tag<Profile> profile = "json:`profile`";
        };
        Tag<User> user = "json:`user`";

        struct Meta {
            Tag<std::string> request_id = "json:`requestId`";
            Tag<std::tm>     timestamp  = "json:`timestamp`";
        };
        Tag<Meta> meta = "json:`meta`";
    };

    struct Schema2 {
        struct User {
            int                      id;
            std::string              username;
            std::string              email;
            std::vector<std::string> roles;

            struct Profile {
                int         age;
                std::string country;

                struct Preferences {
                    bool dark_mode;
                    struct Notifications {
                        bool email;
                        bool push;
                    };
                    Notifications notifications;
                };
                Preferences preferences;
            };
            Profile profile;
        };
        User user;

        struct Meta {
            std::string request_id;
            std::tm     timestamp;
        };
        Meta meta;
    };
} // namespace

static constexpr cpx::TagInfo user = "user";
static constexpr cpx::TagInfo meta = "meta";

template <>
struct cpx::Reflect<Schema2> //
    : Fields<                //
          Field<&Schema2::user, user>,
          Field<&Schema2::meta, meta>> {};

static constexpr cpx::TagInfo id       = "id";
static constexpr cpx::TagInfo username = "username";
static constexpr cpx::TagInfo email    = "email";
static constexpr cpx::TagInfo roles    = "roles";
static constexpr cpx::TagInfo profile  = "profile";

template <>
struct cpx::Reflect<Schema2::User> //
    : Fields<
          Field<&Schema2::User::id, id>,
          Field<&Schema2::User::username, username>,
          Field<&Schema2::User::email, email>,
          Field<&Schema2::User::roles, roles>,
          Field<&Schema2::User::profile, profile>> {};

static constexpr cpx::TagInfo request_id = "requestId";
static constexpr cpx::TagInfo timestamp  = "timestamp";

template <>
struct cpx::Reflect<Schema2::Meta> //
    : Fields<                      //
          Field<&Schema2::Meta::request_id, request_id>,
          Field<&Schema2::Meta::timestamp, timestamp>> {};

static constexpr cpx::TagInfo age         = "age";
static constexpr cpx::TagInfo country     = "country";
static constexpr cpx::TagInfo preferences = "preferences";

template <>
struct cpx::Reflect<Schema2::User::Profile> //
    : Fields<
          Field<&Schema2::User::Profile::age, age>,
          Field<&Schema2::User::Profile::country, country>,
          Field<&Schema2::User::Profile::preferences, preferences>> {};

static constexpr cpx::TagInfo dark_mode     = "darkMode";
static constexpr cpx::TagInfo notifications = "notifications";

template <>
struct cpx::Reflect<Schema2::User::Profile::Preferences> //
    : Fields<
          Field<&Schema2::User::Profile::Preferences::dark_mode, dark_mode>,
          Field<&Schema2::User::Profile::Preferences::notifications, notifications>> {};

static constexpr cpx::TagInfo push = cpx::TagInfoBuilder("push");

template <>
struct cpx::Reflect<Schema2::User::Profile::Preferences::Notifications>
    : cpx::Fields<
          Field<&Schema2::User::Profile::Preferences::Notifications::email, email>,
          Field<&Schema2::User::Profile::Preferences::Notifications::push, push>> {};


static const char *data = R"json({
    "user": {
        "id": 12345,
        "username": "sucipto",
        "email": "sucipto@makmursejahtera.co.id",
        "roles": ["admin", "editor"],
        "profile": {
            "age": 30,
            "country": "Indonesia",
            "preferences": {
                "darkMode": true,
                "notifications": {
                    "email": true,
                    "push": false
                }
            }
        }
    },
    "meta": {
        "requestId": "abc-123-xyz",
        "timestamp": "2026-02-12T10:00:00Z"
    }
})json";

static void cpx_tag_yyjson_serialization(benchmark::State &state) {
    Schema s;
    cpx::json::yy_json::parse(data, s);
    for (auto _ : state) {
        std::ignore = cpx::json::yy_json::dump(s);
    }
}
BENCHMARK(cpx_tag_yyjson_serialization);

static void cpx_tag_rapid_json_serialization(benchmark::State &state) {
    Schema s;
    cpx::json::rapid_json::parse(data, s);
    for (auto _ : state) {
        std::ignore = cpx::json::yy_json::dump(s);
    }
}
BENCHMARK(cpx_tag_rapid_json_serialization);

static void cpx_tag_nlohmann_json_serialization(benchmark::State &state) {
    Schema s = nlohmann::json::parse(data);
    for (auto _ : state) {
        nlohmann::json j = s;
        std::ignore      = j.dump();
    }
}
BENCHMARK(cpx_tag_nlohmann_json_serialization);

static void cpx_tag_yyjson_deserialization(benchmark::State &state) {
    std::string payload = data;
    for (auto _ : state) {
        Schema s;
        cpx::json::yy_json::parse(payload, s);
    }
}
BENCHMARK(cpx_tag_yyjson_deserialization);

static void cpx_tag_rapid_json_deserialization(benchmark::State &state) {
    std::string payload = data;
    for (auto _ : state) {
        Schema s;
        cpx::json::rapid_json::parse(payload, s);
    }
}
BENCHMARK(cpx_tag_rapid_json_deserialization);

static void cpx_tag_nlohmann_json_deserialization(benchmark::State &state) {
    std::string payload = data;
    for (auto _ : state) {
        Schema s = nlohmann::json::parse(payload);
    }
}
BENCHMARK(cpx_tag_nlohmann_json_deserialization);

static void cpx_reflect_yyjson_serialization(benchmark::State &state) {
    Schema2 s;
    cpx::json::yy_json::parse(data, s);
    for (auto _ : state) {
        std::ignore = cpx::json::yy_json::dump(s);
    }
}
BENCHMARK(cpx_reflect_yyjson_serialization);

static void cpx_reflect_rapid_json_serialization(benchmark::State &state) {
    Schema2 s;
    cpx::json::rapid_json::parse(data, s);
    for (auto _ : state) {
        std::ignore = cpx::json::yy_json::dump(s);
    }
}
BENCHMARK(cpx_reflect_rapid_json_serialization);

static void cpx_reflect_nlohmann_json_serialization(benchmark::State &state) {
    Schema2 s = nlohmann::json::parse(data);
    for (auto _ : state) {
        nlohmann::json j = s;
        std::ignore      = j.dump();
    }
}
BENCHMARK(cpx_reflect_nlohmann_json_serialization);

static void cpx_reflect_yyjson_deserialization(benchmark::State &state) {
    std::string payload = data;
    for (auto _ : state) {
        Schema2 s;
        cpx::json::yy_json::parse(payload, s);
    }
}
BENCHMARK(cpx_reflect_yyjson_deserialization);

static void cpx_reflect_rapid_json_deserialization(benchmark::State &state) {
    std::string payload = data;
    for (auto _ : state) {
        Schema2 s;
        cpx::json::rapid_json::parse(payload, s);
    }
}
BENCHMARK(cpx_reflect_rapid_json_deserialization);

static void cpx_reflect_nlohmann_json_deserialization(benchmark::State &state) {
    std::string payload = data;
    for (auto _ : state) {
        Schema2 s = nlohmann::json::parse(payload);
    }
}
BENCHMARK(cpx_reflect_nlohmann_json_deserialization);
