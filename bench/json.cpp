#include <cpx/json/yy_json.h>
#include <cpx/json/rapid_json.h>
#include <cpx/json/nlohmann_json.h>
#include <cpx/reflect.h>
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

template <>
struct cpx::Reflect<Schema2> : Fields<Reflect<Schema2>, &Schema2::user, &Schema2::meta> {
    static constexpr cpx::TagInfo user = "user";
    static constexpr cpx::TagInfo meta = "meta";

    static constexpr tags_type tags() {
        return std::tie(user, meta);
    }
};

template <>
struct cpx::Reflect<Schema2::User> //
    : Fields<
          Reflect<Schema2::User>,
          &Schema2::User::id,
          &Schema2::User::username,
          &Schema2::User::email,
          &Schema2::User::roles,
          &Schema2::User::profile> {
    static constexpr cpx::TagInfo id       = "id";
    static constexpr cpx::TagInfo username = "username";
    static constexpr cpx::TagInfo email    = "email";
    static constexpr cpx::TagInfo roles    = "roles";
    static constexpr cpx::TagInfo profile  = "profile";

    static constexpr tags_type tags() {
        return std::tie(id, username, email, roles, profile);
    }
};

template <>
struct cpx::Reflect<Schema2::Meta> : Fields<Reflect<Schema2::Meta>, &Schema2::Meta::request_id, &Schema2::Meta::timestamp> {
    static constexpr cpx::TagInfo request_id = "requestId";
    static constexpr cpx::TagInfo timestamp  = "timestamp";

    static constexpr tags_type tags() {
        return std::tie(request_id, timestamp);
    }
};

template <>
struct cpx::Reflect<Schema2::User::Profile> //
    : Fields<
          Reflect<Schema2::User::Profile>,
          &Schema2::User::Profile::age,
          &Schema2::User::Profile::country,
          &Schema2::User::Profile::preferences> {
    static constexpr cpx::TagInfo age         = "age";
    static constexpr cpx::TagInfo country     = "country";
    static constexpr cpx::TagInfo preferences = "preferences";

    static constexpr tags_type tags() {
        return std::tie(age, country, preferences);
    }
};

template <>
struct cpx::Reflect<Schema2::User::Profile::Preferences> //
    : Fields<
          Reflect<Schema2::User::Profile::Preferences>,
          &Schema2::User::Profile::Preferences::dark_mode,
          &Schema2::User::Profile::Preferences::notifications> {

    static constexpr cpx::TagInfo dark_mode     = "darkMode";
    static constexpr cpx::TagInfo notifications = "notifications";

    static constexpr tags_type tags() {
        return std::tie(dark_mode, notifications);
    }
};

template <>
struct cpx::Reflect<Schema2::User::Profile::Preferences::Notifications>
    : Fields<
          Reflect<Schema2::User::Profile::Preferences::Notifications>,
          &Schema2::User::Profile::Preferences::Notifications::email,
          &Schema2::User::Profile::Preferences::Notifications::push> {
    static constexpr cpx::TagInfo push = cpx::TagInfoBuilder("push");

    static constexpr tags_type tags() {
        return std::tie(Reflect<Schema2::User>::email, push);
    }
};


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
