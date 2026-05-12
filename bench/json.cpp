#include <cpx/json/yy_json.h>
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
struct cpx::Reflect<Schema2> : cpx::Fields<&Schema2::user, &Schema2::meta> {
    static constexpr cpx::TagInfo user = cpx::TagInfoBuilder("user");
    static constexpr cpx::TagInfo meta = cpx::TagInfoBuilder("meta");

    static const_type of(const Schema2 &p) {
        return std::make_tuple(cpx::tag_tie(p.user, user), cpx::tag_tie(p.meta, meta));
    }

    static type of(Schema2 &p) {
        return std::make_tuple(cpx::tag_tie(p.user, user), cpx::tag_tie(p.meta, meta));
    }
};

template <>
struct cpx::Reflect<Schema2::User> : cpx::Fields<
                                         &Schema2::User::id,
                                         &Schema2::User::username,
                                         &Schema2::User::email,
                                         &Schema2::User::roles,
                                         &Schema2::User::profile> {
    static constexpr cpx::TagInfo id       = cpx::TagInfoBuilder("id");
    static constexpr cpx::TagInfo username = cpx::TagInfoBuilder("username");
    static constexpr cpx::TagInfo email    = cpx::TagInfoBuilder("email");
    static constexpr cpx::TagInfo roles    = cpx::TagInfoBuilder("roles");
    static constexpr cpx::TagInfo profile  = cpx::TagInfoBuilder("profile");

    static const_type of(const Schema2::User &p) {
        return std::make_tuple(
            cpx::tag_tie(p.id, id),
            cpx::tag_tie(p.username, username),
            cpx::tag_tie(p.email, email),
            cpx::tag_tie(p.roles, roles),
            cpx::tag_tie(p.profile, profile)
        );
    }

    static type of(Schema2::User &p) {
        return std::make_tuple(
            cpx::tag_tie(p.id, id),
            cpx::tag_tie(p.username, username),
            cpx::tag_tie(p.email, email),
            cpx::tag_tie(p.roles, roles),
            cpx::tag_tie(p.profile, profile)
        );
    }
};

template <>
struct cpx::Reflect<Schema2::Meta> : cpx::Fields<&Schema2::Meta::request_id, &Schema2::Meta::timestamp> {
    static constexpr cpx::TagInfo request_id = cpx::TagInfoBuilder("requestId");
    static constexpr cpx::TagInfo timestamp  = cpx::TagInfoBuilder("timestamp");

    static const_type of(const Schema2::Meta &p) {
        return std::make_tuple(cpx::tag_tie(p.request_id, request_id), cpx::tag_tie(p.timestamp, timestamp));
    }

    static type of(Schema2::Meta &p) {
        return std::make_tuple(cpx::tag_tie(p.request_id, request_id), cpx::tag_tie(p.timestamp, timestamp));
    }
};

template <>
struct cpx::Reflect<Schema2::User::Profile>
    : cpx::Fields<&Schema2::User::Profile::age, &Schema2::User::Profile::country, &Schema2::User::Profile::preferences> {
    static constexpr cpx::TagInfo age         = cpx::TagInfoBuilder("age");
    static constexpr cpx::TagInfo country     = cpx::TagInfoBuilder("country");
    static constexpr cpx::TagInfo preferences = cpx::TagInfoBuilder("preferences");

    static const_type of(const Schema2::User::Profile &p) {
        return std::make_tuple(
            cpx::tag_tie(p.age, age), cpx::tag_tie(p.country, country), cpx::tag_tie(p.preferences, preferences)
        );
    }

    static type of(Schema2::User::Profile &p) {
        return std::make_tuple(
            cpx::tag_tie(p.age, age), cpx::tag_tie(p.country, country), cpx::tag_tie(p.preferences, preferences)
        );
    }
};

template <>
struct cpx::Reflect<Schema2::User::Profile::Preferences>
    : cpx::Fields<&Schema2::User::Profile::Preferences::dark_mode, &Schema2::User::Profile::Preferences::notifications> {
    static constexpr cpx::TagInfo dark_mode     = cpx::TagInfoBuilder("darkMode");
    static constexpr cpx::TagInfo notifications = cpx::TagInfoBuilder("notifications");

    static const_type of(const Schema2::User::Profile::Preferences &p) {
        return std::make_tuple(cpx::tag_tie(p.dark_mode, dark_mode), cpx::tag_tie(p.notifications, notifications));
    }

    static type of(Schema2::User::Profile::Preferences &p) {
        return std::make_tuple(cpx::tag_tie(p.dark_mode, dark_mode), cpx::tag_tie(p.notifications, notifications));
    }
};

template <>
struct cpx::Reflect<Schema2::User::Profile::Preferences::Notifications>
    : cpx::Fields<
          &Schema2::User::Profile::Preferences::Notifications::email,
          &Schema2::User::Profile::Preferences::Notifications::push> {
    static constexpr cpx::TagInfo email = cpx::TagInfoBuilder("email");
    static constexpr cpx::TagInfo push  = cpx::TagInfoBuilder("push");

    static const_type of(const Schema2::User::Profile::Preferences::Notifications &p) {
        return std::make_tuple(cpx::tag_tie(p.email, email), cpx::tag_tie(p.push, push));
    }

    static type of(Schema2::User::Profile::Preferences::Notifications &p) {
        return std::make_tuple(cpx::tag_tie(p.email, email), cpx::tag_tie(p.push, push));
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

static void cpx_reflect_nlohmann_json_deserialization(benchmark::State &state) {
    std::string payload = data;
    for (auto _ : state) {
        Schema2 s = nlohmann::json::parse(payload);
    }
}
BENCHMARK(cpx_reflect_nlohmann_json_deserialization);
