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
struct cpx::serde::SerializeAs<cpx::json::JsonGeneric, Schema2> : std::true_type {
    static constexpr cpx::TagInfo user = cpx::TagInfoBuilder("user");
    static constexpr cpx::TagInfo meta = cpx::TagInfoBuilder("meta");

    const Schema2 &p;

    SerializeAs(const Schema2 &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<const decltype(Schema2::user) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Schema2::meta) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(std::tie(p.user, user), std::tie(p.meta, meta));
    }
};

template <>
struct cpx::serde::DeserializeAs<cpx::json::JsonGeneric, Schema2> : std::true_type {
    static constexpr cpx::TagInfo user = cpx::TagInfoBuilder("user");
    static constexpr cpx::TagInfo meta = cpx::TagInfoBuilder("meta");

    Schema2 &p;

    DeserializeAs(Schema2 &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<decltype(Schema2::user) &, const cpx::TagInfo &>,
        std::tuple<decltype(Schema2::meta) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(std::tie(p.user, user), std::tie(p.meta, meta));
    }
};

template <>
struct cpx::serde::SerializeAs<cpx::json::JsonGeneric, Schema2::User> : std::true_type {
    static constexpr cpx::TagInfo id       = cpx::TagInfoBuilder("id");
    static constexpr cpx::TagInfo username = cpx::TagInfoBuilder("username");
    static constexpr cpx::TagInfo email    = cpx::TagInfoBuilder("email");
    static constexpr cpx::TagInfo roles    = cpx::TagInfoBuilder("roles");
    static constexpr cpx::TagInfo profile  = cpx::TagInfoBuilder("profile");

    const Schema2::User &p;

    SerializeAs(const Schema2::User &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<const decltype(Schema2::User::id) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Schema2::User::username) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Schema2::User::email) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Schema2::User::roles) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Schema2::User::profile) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(
            std::tie(p.id, id),
            std::tie(p.username, username),
            std::tie(p.email, email),
            std::tie(p.roles, roles),
            std::tie(p.profile, profile)
        );
    }
};

template <>
struct cpx::serde::DeserializeAs<cpx::json::JsonGeneric, Schema2::User> : std::true_type {
    static constexpr cpx::TagInfo id       = cpx::TagInfoBuilder("id");
    static constexpr cpx::TagInfo username = cpx::TagInfoBuilder("username");
    static constexpr cpx::TagInfo email    = cpx::TagInfoBuilder("email");
    static constexpr cpx::TagInfo roles    = cpx::TagInfoBuilder("roles");
    static constexpr cpx::TagInfo profile  = cpx::TagInfoBuilder("profile");

    Schema2::User &p;

    DeserializeAs(Schema2::User &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<decltype(Schema2::User::id) &, const cpx::TagInfo &>,
        std::tuple<decltype(Schema2::User::username) &, const cpx::TagInfo &>,
        std::tuple<decltype(Schema2::User::email) &, const cpx::TagInfo &>,
        std::tuple<decltype(Schema2::User::roles) &, const cpx::TagInfo &>,
        std::tuple<decltype(Schema2::User::profile) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(
            std::tie(p.id, id),
            std::tie(p.username, username),
            std::tie(p.email, email),
            std::tie(p.roles, roles),
            std::tie(p.profile, profile)
        );
    }
};

template <>
struct cpx::serde::SerializeAs<cpx::json::JsonGeneric, Schema2::Meta> : std::true_type {
    static constexpr cpx::TagInfo request_id = cpx::TagInfoBuilder("requestId");
    static constexpr cpx::TagInfo timestamp  = cpx::TagInfoBuilder("timestamp");

    const Schema2::Meta &p;

    SerializeAs(const Schema2::Meta &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<const decltype(Schema2::Meta::request_id) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Schema2::Meta::timestamp) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(std::tie(p.request_id, request_id), std::tie(p.timestamp, timestamp));
    }
};

template <>
struct cpx::serde::DeserializeAs<cpx::json::JsonGeneric, Schema2::Meta> : std::true_type {
    static constexpr cpx::TagInfo request_id = cpx::TagInfoBuilder("requestId");
    static constexpr cpx::TagInfo timestamp  = cpx::TagInfoBuilder("timestamp");

    Schema2::Meta &p;

    DeserializeAs(Schema2::Meta &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<decltype(Schema2::Meta::request_id) &, const cpx::TagInfo &>,
        std::tuple<decltype(Schema2::Meta::timestamp) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(std::tie(p.request_id, request_id), std::tie(p.timestamp, timestamp));
    }
};

template <>
struct cpx::serde::SerializeAs<cpx::json::JsonGeneric, Schema2::User::Profile> : std::true_type {
    static constexpr cpx::TagInfo age         = cpx::TagInfoBuilder("age");
    static constexpr cpx::TagInfo country     = cpx::TagInfoBuilder("country");
    static constexpr cpx::TagInfo preferences = cpx::TagInfoBuilder("preferences");

    const Schema2::User::Profile &p;

    SerializeAs(const Schema2::User::Profile &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<const decltype(Schema2::User::Profile::age) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Schema2::User::Profile::country) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Schema2::User::Profile::preferences) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(std::tie(p.age, age), std::tie(p.country, country), std::tie(p.preferences, preferences));
    }
};

template <>
struct cpx::serde::DeserializeAs<cpx::json::JsonGeneric, Schema2::User::Profile> : std::true_type {
    static constexpr cpx::TagInfo age         = cpx::TagInfoBuilder("age");
    static constexpr cpx::TagInfo country     = cpx::TagInfoBuilder("country");
    static constexpr cpx::TagInfo preferences = cpx::TagInfoBuilder("preferences");

    Schema2::User::Profile &p;

    DeserializeAs(Schema2::User::Profile &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<decltype(Schema2::User::Profile::age) &, const cpx::TagInfo &>,
        std::tuple<decltype(Schema2::User::Profile::country) &, const cpx::TagInfo &>,
        std::tuple<decltype(Schema2::User::Profile::preferences) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(std::tie(p.age, age), std::tie(p.country, country), std::tie(p.preferences, preferences));
    }
};

template <>
struct cpx::serde::SerializeAs<cpx::json::JsonGeneric, Schema2::User::Profile::Preferences> : std::true_type {
    static constexpr cpx::TagInfo dark_mode     = cpx::TagInfoBuilder("darkMode");
    static constexpr cpx::TagInfo notifications = cpx::TagInfoBuilder("notifications");

    const Schema2::User::Profile::Preferences &p;

    SerializeAs(const Schema2::User::Profile::Preferences &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<const decltype(Schema2::User::Profile::Preferences::dark_mode) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Schema2::User::Profile::Preferences::notifications) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(std::tie(p.dark_mode, dark_mode), std::tie(p.notifications, notifications));
    }
};

template <>
struct cpx::serde::DeserializeAs<cpx::json::JsonGeneric, Schema2::User::Profile::Preferences> : std::true_type {
    static constexpr cpx::TagInfo dark_mode     = cpx::TagInfoBuilder("darkMode");
    static constexpr cpx::TagInfo notifications = cpx::TagInfoBuilder("notifications");

    Schema2::User::Profile::Preferences &p;

    DeserializeAs(Schema2::User::Profile::Preferences &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<decltype(Schema2::User::Profile::Preferences::dark_mode) &, const cpx::TagInfo &>,
        std::tuple<decltype(Schema2::User::Profile::Preferences::notifications) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(std::tie(p.dark_mode, dark_mode), std::tie(p.notifications, notifications));
    }
};

template <>
struct cpx::serde::SerializeAs<cpx::json::JsonGeneric, Schema2::User::Profile::Preferences::Notifications> : std::true_type {
    static constexpr cpx::TagInfo email = cpx::TagInfoBuilder("email");
    static constexpr cpx::TagInfo push  = cpx::TagInfoBuilder("push");

    const Schema2::User::Profile::Preferences::Notifications &p;

    SerializeAs(const Schema2::User::Profile::Preferences::Notifications &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<const decltype(Schema2::User::Profile::Preferences::Notifications::email) &, const cpx::TagInfo &>,
        std::tuple<const decltype(Schema2::User::Profile::Preferences::Notifications::push) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(std::tie(p.email, email), std::tie(p.push, push));
    }
};

template <>
struct cpx::serde::DeserializeAs<cpx::json::JsonGeneric, Schema2::User::Profile::Preferences::Notifications> : std::true_type {
    static constexpr cpx::TagInfo email = cpx::TagInfoBuilder("email");
    static constexpr cpx::TagInfo push  = cpx::TagInfoBuilder("push");

    Schema2::User::Profile::Preferences::Notifications &p;

    DeserializeAs(Schema2::User::Profile::Preferences::Notifications &p)
        : p(p) {}

    using type = std::tuple<
        std::tuple<decltype(Schema2::User::Profile::Preferences::Notifications::email) &, const cpx::TagInfo &>,
        std::tuple<decltype(Schema2::User::Profile::Preferences::Notifications::push) &, const cpx::TagInfo &>>;

    operator type() const {
        return std::make_tuple(std::tie(p.email, email), std::tie(p.push, push));
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
    Schema s = nlohmann::json::parse(data);
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
