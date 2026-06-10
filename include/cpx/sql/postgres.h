#ifndef CPX_SQL_POSTGRES_H
#define CPX_SQL_POSTGRES_H

#include <cpx/sql/sql.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/serde/error.h>
#include <cpx/time.h>
#include <cstring>
#include <netinet/in.h>

#if __has_include(<postgresql/libpq-fe.h>)
#    include <postgresql/libpq-fe.h>
#elif __has_include(<libpq-fe.h>)
#    include <postgresql/libpq-fe.h>
#else
#    error "Cannot find libpq-fe.h"
#endif


namespace cpx::sql::postgres {
    struct Value {
        const char *buffer;
        int         length;
        int         format; // 0=text, 1=binary
        Oid         type;
        char        small_buffer[8];
    };

    class Connection;

    template <typename Row>
    class Rows;

    template <typename From, typename Enable = void>
    using Serialize = ::cpx::serde::Serialize<PGresult, From, Enable>;

    template <typename To, typename Enable = void>
    using Deserialize = ::cpx::serde::Deserialize<PGresult, To, Enable>;

    template <typename T>
    struct OID;

    template <>
    struct OID<bool> {
        static constexpr Oid              value = 16;
        static constexpr std::string_view name  = "bool";
    };

    template <>
    struct OID<int16_t> {
        static constexpr Oid              value = 21;
        static constexpr std::string_view name  = "int2";
    };

    template <>
    struct OID<int32_t> {
        static constexpr Oid              value = 23;
        static constexpr std::string_view name  = "int4";
    };

    template <>
    struct OID<int64_t> {
        static constexpr Oid              value = 20;
        static constexpr std::string_view name  = "int8";
    };

    template <>
    struct OID<float> {
        static constexpr Oid              value = 700;
        static constexpr std::string_view name  = "float4";
    };

    template <>
    struct OID<double> {
        static constexpr Oid              value = 701;
        static constexpr std::string_view name  = "float8";
    };

    template <>
    struct OID<std::string_view> {
        static constexpr Oid              value = 25;
        static constexpr Oid              bytea = 17;
        static constexpr std::string_view name  = "text";
    };

    template <>
    struct OID<std::timespec> {
        static constexpr Oid              value = 1184;
        static constexpr std::string_view name  = "timestamptz";
    };
} // namespace cpx::sql::postgres

namespace cpx {
    namespace postgres = cpx::sql::postgres;
}

namespace cpx::sql::postgres::detail {
    std::string convert_placeholders(const std::string &query) {
        std::string result;
        result.reserve(query.size() + 16); // small optimization

        int counter = 1;
        for (char c : query) {
            if (c == '?') {
                result += "$" + std::to_string(counter++);
            } else {
                result.push_back(c);
            }
        }
        return result;
    }
} // namespace cpx::sql::postgres::detail


namespace cpx::sql::postgres {
    template <typename Row>
    class Rows : public cpx::sql::Rows<Row> {
        friend class Connection;

    protected:
        Rows(PGconn *db, bool is_binary = true)
            : db(db)
            , is_binary(is_binary) {
            try {
                next();
            } catch (std::runtime_error &e) {
                if (res)
                    PQclear(res);
                while (PQgetResult(db))
                    ;
                throw;
            }
        }

    public:
        Rows(const Rows &)            = delete;
        Rows &operator=(const Rows &) = delete;

        Rows(Rows &&other) noexcept
            : db(other.db)
            , res(std::exchange(other.res, nullptr))
            , is_binary(other.is_binary) {}

        Rows &operator=(Rows &&) noexcept = delete;

        ~Rows() override {
            if (res)
                PQclear(res);
            while (PQgetResult(db))
                ;
        }

        void next() override {
            if (res) {
                PQclear(res);
                res = nullptr;
            }

            while ((res = PQgetResult(db))) {
                auto status = PQresultStatus(res);

                if (status == PGRES_SINGLE_TUPLE)
                    return;

                if (status == PGRES_TUPLES_OK || status == PGRES_COMMAND_OK) {
                    PQclear(res);
                    res = nullptr;
                    continue;
                }

                std::string msg = PQresultErrorMessage(res);
                PQclear(res);
                res = nullptr;
                throw std::runtime_error(msg);
            }
        }

        Row get() const override {
            if (res == nullptr)
                throw std::runtime_error("Failed to get row data: not a row");

            Row row = {};
            cpx::tuple_for_each(row, [&](auto &item, size_t i) {
                const int  row     = 0;
                const int  col     = int(i);
                const bool is_null = PQgetisnull(res, row, col);

                Value val;
                val.buffer = PQgetvalue(res, row, col);
                val.length = PQgetlength(res, row, col);
                val.type   = PQftype(res, col);
                val.format = PQfformat(res, col);
                try {
                    Deserialize<std::decay_t<decltype(item)>>{val, is_null}.into(item);
                } catch (cpx::serde::error &e) {
                    std::string ctx = "row=" + std::to_string(row) + ",col=" + std::to_string(col);
                    e.add_context(ctx);
                    throw;
                }
            });

            return row;
        }

        bool is_done() const override {
            return res == nullptr;
        }

    protected:
        PGconn   *db;
        PGresult *res = nullptr;
        bool      is_binary;
    };

    class Connection : public cpx::sql::Connection {
    public:
        Connection(const std::string &spec) {
            db = PQconnectdb(spec.c_str());
            if (PQstatus(db) != CONNECTION_OK) {
                std::string what = PQerrorMessage(db);
                PQfinish(db);
                std::string msg = "Cannot connect with spec=\"" + spec + "\": " + what;
                throw std::runtime_error(msg);
            }
        }

        ~Connection() override {
            PQfinish(db);
        }

        template <typename Params, typename Row>
        Rows<Row> operator()(const Statement<Params, Row> &statement, bool is_binary = true) {
            std::vector<cpx::sql::postgres::Value> doc;
            doc.reserve(std::tuple_size_v<Params>);

            Serialize<Params>{doc}.from(statement.params);

            std::vector<Oid>          types(doc.size());
            std::vector<const char *> values(doc.size());
            std::vector<int>          lengths(doc.size());
            std::vector<int>          formats(doc.size());
            for (size_t i = 0; i < doc.size(); ++i) {
                types[i]   = doc[i].type;
                values[i]  = doc[i].buffer ? doc[i].buffer : doc[i].small_buffer;
                lengths[i] = doc[i].length;
                formats[i] = doc[i].format;
            }

            std::string converted = detail::convert_placeholders(statement.query);

            int res = PQsendQueryParams(
                db,
                converted.c_str(),
                (int)doc.size(),
                types.data(),
                values.data(),
                lengths.data(),
                formats.data(),
                is_binary ? 1 : 0
            );
            if (!res)
                throw std::runtime_error(std::string("PQsendQueryParams: ") + PQerrorMessage(db));

            res = PQsetSingleRowMode(db);
            if (!res)
                throw std::runtime_error(std::string("PQsetSingleRowMode: ") + PQerrorMessage(db));

            return {db, is_binary};
        }

        void begin_transaction() override {
            PGresult *res = PQexec(db, "BEGIN");
            check(res, "BEGIN");
        }

        void commit() override {
            PGresult *res = PQexec(db, "COMMIT");
            check(res, "COMMIT");
        }

        void cancel() override {
            PGresult *res = PQexec(db, "ROLLBACK");
            check(res, "ROLLBACK");
        }

    protected:
        PGconn *db;

        void check(PGresult *res, const std::string &query) const {
            if (auto status = PQresultStatus(res); status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK) {
                std::string what = PQerrorMessage(db);
                PQclear(res);
                std::string msg = "Failed to execute statement \"" + query + "\": " + std::to_string(status) + ": " + what;
                throw std::runtime_error(msg);
            }
        }
    };
} // namespace cpx::sql::postgres


namespace cpx::sql::postgres::detail {
    inline uint16_t bswap16(uint16_t v) {
#if defined(_MSC_VER)
        return _byteswap_ushort(v);
#elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap16(v);
#else
        return (v >> 8) | (v << 8);
#endif
    }

    inline uint32_t bswap32(uint32_t v) {
#if defined(_MSC_VER)
        return _byteswap_ulong(v);
#elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap32(v);
#else
        return ((v & 0x000000FFu) << 24) | ((v & 0x0000FF00u) << 8) | ((v & 0x00FF0000u) >> 8) | ((v & 0xFF000000u) >> 24);
#endif
    }

    inline uint64_t bswap64(uint64_t v) {
#if defined(_MSC_VER)
        return _byteswap_uint64(v);
#elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap64(v);
#else
        return ((v & 0x00000000000000FFull) << 56) | ((v & 0x000000000000FF00ull) << 40) | ((v & 0x0000000000FF0000ull) << 24) |
               ((v & 0x00000000FF000000ull) << 8) | ((v & 0x000000FF00000000ull) >> 8) | ((v & 0x0000FF0000000000ull) >> 24) |
               ((v & 0x00FF000000000000ull) >> 40) | ((v & 0xFF00000000000000ull) >> 56);
#endif
    }

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    constexpr bool native_big_endian = true;
#else
    constexpr bool native_big_endian = false;
#endif

    template <typename T>
    inline T byteswap(T value) {
        if constexpr (sizeof(T) == 2) {
            return static_cast<T>(bswap16(static_cast<uint16_t>(value)));
        }

        if constexpr (sizeof(T) == 4) {
            return static_cast<T>(bswap32(static_cast<uint32_t>(value)));
        }

        if constexpr (sizeof(T) == 8) {
            return static_cast<T>(bswap64(static_cast<uint64_t>(value)));
        }
    }

    template <typename T>
    inline T host_to_network(T value) {
        if constexpr (native_big_endian)
            return value;
        return byteswap(value);
    }

    template <typename T>
    inline T network_to_host(T value) {
        return host_to_network(value);
    }


    // ---------------- integers ----------------
    inline void into_network(int16_t value, char *buffer) {
        value = host_to_network(value);
        std::memcpy(buffer, &value, sizeof(value));
    }

    inline void into_network(int32_t value, char *buffer) {
        value = host_to_network(value);
        std::memcpy(buffer, &value, sizeof(value));
    }

    inline void into_network(int64_t value, char *buffer) {
        value = host_to_network(value);
        std::memcpy(buffer, &value, sizeof(value));
    }

    inline void from_network(int16_t &value, const char *buffer) {
        std::memcpy(&value, buffer, sizeof(value));
        value = network_to_host(value);
    }

    inline void from_network(int32_t &value, const char *buffer) {
        std::memcpy(&value, buffer, sizeof(value));
        value = network_to_host(value);
    }

    inline void from_network(int64_t &value, const char *buffer) {
        std::memcpy(&value, buffer, sizeof(value));
        value = network_to_host(value);
    }

    // ---------------- float / double ----------------
    inline void into_network(float value, char *buffer) {
        static_assert(sizeof(float) == sizeof(uint32_t));
        uint32_t bits;
        std::memcpy(&bits, &value, sizeof(bits));
        bits = host_to_network(bits);
        std::memcpy(buffer, &bits, sizeof(bits));
    }

    inline void into_network(double value, char *buffer) {
        static_assert(sizeof(double) == sizeof(uint64_t));
        uint64_t bits;
        std::memcpy(&bits, &value, sizeof(bits));
        bits = host_to_network(bits);
        std::memcpy(buffer, &bits, sizeof(bits));
    }

    inline void from_network(float &value, const char *buffer) {
        uint32_t bits;
        std::memcpy(&bits, buffer, sizeof(bits));
        bits = network_to_host(bits);
        std::memcpy(&value, &bits, sizeof(value));
    }

    inline void from_network(double &value, const char *buffer) {
        uint64_t bits;
        std::memcpy(&bits, buffer, sizeof(bits));
        bits = network_to_host(bits);
        std::memcpy(&value, &bits, sizeof(value));
    }

    // ---------------- bool ----------------
    inline void into_network(bool value, char *buffer) {
        buffer[0] = value ? 1 : 0;
    }
    inline void from_network(bool &value, const char *buffer) {
        value = buffer[0] != 0;
    }
} // namespace cpx::sql::postgres::detail

#define SERIALIZE(...)      cpx::serde::Serialize<PGresult, __VA_ARGS__>
#define DESERIALIZE(...)    cpx::serde::Deserialize<PGresult, __VA_ARGS__>
#define SERIALIZABLE(...)   cpx::serde::is_serializable_v<PGresult, __VA_ARGS__>
#define DESERIALIZABLE(...) cpx::serde::is_deserializable_v<PGresult, __VA_ARGS__>

#define SERIALIZER_FIELDS std::vector<cpx::sql::postgres::Value> &doc;

#define DESERIALIZER_FIELDS                                                                                                      \
    const cpx::sql::postgres::Value &val;                                                                                        \
    bool                             is_null;

template <typename T>
struct SERIALIZE(T, std::enable_if_t<std::is_same_v<T, bool> || (std::is_signed_v<T> && (sizeof(T) > 1))>) {
    SERIALIZER_FIELDS

    void from(T value, Oid type = 0) {
        cpx::sql::postgres::Value val;

        val.type   = type == 0 ? cpx::sql::postgres::OID<T>::value : type;
        val.buffer = nullptr;
        val.length = sizeof(T);
        val.format = 1;
        cpx::sql::postgres::detail::into_network(value, val.small_buffer);
        doc.push_back(val);
    }
};

template <typename CT>
struct SERIALIZE(std::basic_string_view<char, CT>) {
    SERIALIZER_FIELDS

    void from(std::basic_string_view<char, CT> value, bool is_blob = false) const {
        doc.push_back({value.data(), (int)value.size(), is_blob ? 1 : 0, is_blob ? 17u : 0u});
    }
};

template <typename CT, typename A>
struct SERIALIZE(std::basic_string<char, CT, A>) {
    SERIALIZER_FIELDS

    void from(const std::basic_string<char, CT, A> &value, bool is_blob = false) const {
        SERIALIZE(std::basic_string_view<char, CT>){doc}.from(value, is_blob);
    }
};

template <typename A>
struct SERIALIZE(std::vector<uint8_t, A>) {
    SERIALIZER_FIELDS

    void from(const std::vector<uint8_t, A> &value) const {
        SERIALIZE(std::string_view){doc}.from(std::string_view(reinterpret_cast<const char *>(value.data()), value.size()), true);
    }
};

template <>
struct SERIALIZE(std::timespec) {
    SERIALIZER_FIELDS

    void from(std::timespec value) const {
        constexpr int64_t POSTGRES_EPOCH_DIFF_SECONDS = 946684800; // 1970 -> 2000

        int64_t pg_micros = (value.tv_sec - POSTGRES_EPOCH_DIFF_SECONDS) * 1'000'000 + value.tv_nsec / 1'000;

        SERIALIZE(int64_t){doc}.from(pg_micros, cpx::sql::postgres::OID<std::timespec>::value);
    }
};

template <>
struct SERIALIZE(std::tm) {
    SERIALIZER_FIELDS

    void from(std::tm value) const {
#if defined(_WIN32)
        auto t = _mkgmtime(&value);
#else
        auto t = ::timegm(&value);
#endif

        SERIALIZE(std::timespec){doc}.from({t, 0});
    }
};

template <typename T>
struct SERIALIZE(std::optional<T>, std::enable_if_t<SERIALIZABLE(T)>) {
    SERIALIZER_FIELDS

    void from(const std::optional<T> &value) const {
        if (!value.has_value())
            doc.push_back({nullptr, 0, 0, 0, {}});
        else
            SERIALIZE(T){doc}.from(*value);
    }

    void from(std::optional<T> &&value) const = delete;
};

template <typename T>
struct SERIALIZE(std::vector<T>, std::enable_if_t<SERIALIZABLE(T) && !std::is_same_v<T, uint8_t>>) {
    SERIALIZER_FIELDS

    void from(const std::vector<T> &value) const {
        for (auto &v : value)
            SERIALIZE(T){doc}.from(v);
    }

    void from(std::vector<T> &&value) const = delete;
};

template <typename... Ts>
struct SERIALIZE(std::tuple<Ts...>, std::enable_if_t<(SERIALIZABLE(Ts) && ...)>) {
    SERIALIZER_FIELDS

    void from(const std::tuple<Ts...> &value) const {
        std::apply([&](auto &&...args) { (SERIALIZE(Ts){doc}.from(args), ...); }, value);
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<std::is_same_v<T, bool> || (std::is_signed_v<T> && (sizeof(T) > 1))>) {
    DESERIALIZER_FIELDS

    void into(T &value) const {
        using namespace cpx::sql::postgres;

        if (is_null)
            throw type_mismatch_error(std::string(OID<T>::name), "null");

        if (val.format == 0 || val.type == OID<std::string_view>::value) {
            std::string str(val.buffer, (size_t)val.length);
            if constexpr (std::is_same_v<T, bool>) {
                value = (str == "true" || str == "1");
            } else if constexpr (std::is_same_v<T, int16_t> || std::is_same_v<T, int32_t>) {
                value = std::stoi(str);
            } else if constexpr (std::is_same_v<T, int64_t>) {
                value = std::stol(str);
            } else if constexpr (std::is_same_v<T, float>) {
                value = std::stof(str);
            } else if constexpr (std::is_same_v<T, double>) {
                value = std::stod(str);
            }
        } else if (val.type == OID<bool>::value) {
            bool v;
            detail::from_network(v, val.buffer);
            value = T(v);
        } else if (val.type == OID<int16_t>::value) {
            int16_t v;
            detail::from_network(v, val.buffer);
            value = T(v);
        } else if (val.type == OID<int32_t>::value) {
            int32_t v;
            detail::from_network(v, val.buffer);
            value = T(v);
        } else if (val.type == OID<int64_t>::value || val.type == OID<std::timespec>::value) {
            int64_t v;
            detail::from_network(v, val.buffer);
            value = T(v);
        } else if (val.type == OID<float>::value) {
            float v;
            detail::from_network(v, val.buffer);
            value = T(v);
        } else if (val.type == OID<double>::value) {
            double v;
            detail::from_network(v, val.buffer);
            value = T(v);
        } else if (val.type == OID<std::string_view>::value) {
            std::string str(val.buffer, (size_t)val.length);
            if constexpr (std::is_same_v<T, bool>) {
                value = (str == "true" || str == "1");
            } else if constexpr (std::is_same_v<T, int16_t> || std::is_same_v<T, int32_t>) {
                value = std::stoi(str);
            } else if constexpr (std::is_same_v<T, int64_t>) {
                value = std::stol(str);
            } else if constexpr (std::is_same_v<T, float>) {
                value = std::stof(str);
            } else if constexpr (std::is_same_v<T, double>) {
                value = std::stod(str);
            }
        } else {
            throw type_mismatch_error(std::string(OID<T>::name), "oid=" + std::to_string(val.type));
        }
    }
};

template <typename CT>
struct DESERIALIZE(std::basic_string_view<char, CT>) {
    DESERIALIZER_FIELDS

    void into(std::basic_string_view<char, CT> &value, bool is_blob = false) const {
        using namespace cpx::sql::postgres;

        const auto expect      = is_blob ? OID<std::string_view>::bytea : OID<std::string_view>::value;
        const auto expect_name = is_blob ? "bytea" : OID<std::string_view>::name;
        if (is_null)
            throw type_mismatch_error(std::string(expect_name), "null");
        else if ((!is_blob && val.format == 0) || val.type == expect)
            value = std::basic_string_view<char, CT>(val.buffer, (size_t)val.length);
        else if (is_blob && val.format == 0)
            throw error("TODO: need b64 decoding");
        else
            throw type_mismatch_error(std::string(expect_name), "oid=" + std::to_string(val.type));
    }
};

template <typename CT, typename A>
struct DESERIALIZE(std::basic_string<char, CT, A>) {
    DESERIALIZER_FIELDS

    void into(std::basic_string<char, CT, A> &value, bool is_blob = false) const {
        std::basic_string_view<char, CT> str;
        DESERIALIZE(decltype(str)){val, is_null}.into(str, is_blob);
        value = std::basic_string<char, CT, A>(str);
    }
};

template <typename A>
struct DESERIALIZE(std::vector<uint8_t, A>) {
    DESERIALIZER_FIELDS

    void into(std::vector<uint8_t, A> &value) const {
        std::string_view str;
        DESERIALIZE(decltype(str)){val, is_null}.into(str, true);
        value = std::vector<uint8_t, A>(str.begin(), str.end());
    }
};

template <>
struct DESERIALIZE(std::timespec) {
    DESERIALIZER_FIELDS

    void into(std::timespec &value) const {
        using namespace cpx::sql::postgres;
        if (is_null) {
            throw type_mismatch_error(std::string(OID<std::timespec>::name), "null");
        }

        if (val.format == 0 || val.type == OID<std::string_view>::value) {
            std::string str(val.buffer, size_t(val.length));
            value = cpx::ts_from_string(str);
        } else if (val.type == OID<std::timespec>::value) {
            int64_t micros;
            DESERIALIZE(int64_t){val, is_null}.into(micros);

            constexpr int64_t PG_TO_UNIX_SECONDS = 946684800LL; // 1970 -> 2000

            int64_t sec    = micros / 1'000'000;
            int64_t rem_us = micros % 1'000'000;

            // fix negative modulo
            if (rem_us < 0) {
                rem_us += 1'000'000;
                --sec;
            }

            value.tv_sec  = sec + PG_TO_UNIX_SECONDS;
            value.tv_nsec = rem_us * 1000;
        } else {
            throw type_mismatch_error(std::string(OID<std::timespec>::name), "oid=" + std::to_string(val.type));
        }
    }
};

template <>
struct DESERIALIZE(std::tm) {
    DESERIALIZER_FIELDS

    void into(std::tm &value) const {
        std::timespec t;
        DESERIALIZE(std::timespec){val, is_null}.into(t);
        value = *std::gmtime(&t.tv_sec);
    }
};

template <typename T>
struct DESERIALIZE(std::optional<T>, std::enable_if_t<DESERIALIZABLE(T)>) {
    DESERIALIZER_FIELDS

    void into(std::optional<T> &value) const {
        if (is_null) {
            value = std::nullopt;
        } else {
            value.emplace();
            DESERIALIZE(T){val, is_null}.into(*value);
        }
    }
};

#undef SERIALIZE
#undef DESERIALIZE
#undef SERIALIZABLE
#undef DESERIALIZABLE
#undef SERIALIZER_FIELDS
#undef DESERIALIZER_FIELDS
#endif
