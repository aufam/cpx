#ifndef CPX_SQL_POSTGRES_H
#define CPX_SQL_POSTGRES_H

#include <cpx/sql/sql.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/serde/error.h>
#include <cpx/defer.h>
#include <cstring>
#include <netinet/in.h>
#include <postgresql/libpq-fe.h>
#include <string>
#include <optional>


namespace cpx::sql::postgres {
    struct Value {
        const char *buffer;
        int         length;
        int         format; // 0=text, 1=binary
        Oid         type;
        bool        is_dynamic = false;
    };

    class Connection;

    template <typename Row>
    class Rows;

    template <typename From, typename Enable = void>
    using Serialize = ::cpx::serde::Serialize<PGresult, From, Enable>;

    template <typename To, typename Enable = void>
    using Deserialize = ::cpx::serde::Deserialize<PGresult, To, Enable>;
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
        Rows(PGresult *res, int row_count, int pq_result_format = 0)
            : res(res)
            , row_count(row_count)
            , pq_result_format(pq_result_format) {}

    public:
        ~Rows() override {
            PQclear(res);
        }

        void next() override {
            ++current_row;
        }

        Row get() const override {
            if (current_row >= row_count)
                throw std::runtime_error("Failed to get row data: not a row");

            Row row = {};
            cpx::tuple_for_each(row, [&](auto &item, size_t i) {
                Deserialize<std::decay_t<decltype(item)>>{res, current_row, int(i), pq_result_format}.into(item);
            });
            return row;
        }

        bool is_done() const override {
            return current_row >= row_count;
        }

    protected:
        PGresult *res;
        int       row_count;
        int       current_row{};
        int       pq_result_format;
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

        template <typename Params, typename Row>
        Rows<Row> operator()(const Statement<Params, Row> &statement, int pq_result_format = 0) {
            std::vector<cpx::sql::postgres::Value> doc;
            doc.reserve(std::tuple_size_v<Params>);
            auto _ = cpx::defer([&]() {
                for (auto &val : doc)
                    if (val.is_dynamic)
                        std::free(const_cast<char *>(val.buffer));
            });

            cpx::tuple_for_each(statement.params, [&](auto &item, size_t) {
                Serialize<std::decay_t<decltype(item)>>{doc}.from(item);
            });

            std::vector<Oid>          types(doc.size());
            std::vector<const char *> values(doc.size());
            std::vector<int>          lengths(doc.size());
            std::vector<int>          formats(doc.size());
            for (size_t i = 0; i < doc.size(); ++i) {
                types[i]   = doc[i].type;
                values[i]  = doc[i].buffer;
                lengths[i] = doc[i].length;
                formats[i] = doc[i].format;
            }

            std::string converted = detail::convert_placeholders(statement.query);
            PGresult   *res       = PQexecParams(
                db,
                converted.c_str(),
                (int)doc.size(),
                types.data(),
                values.data(),
                lengths.data(),
                formats.data(),
                pq_result_format
            );
            check(res, converted);

            int row_count = PQntuples(res);
            return {res, row_count, pq_result_format};
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

        ~Connection() override {
            PQfinish(db);
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
    inline uint8_t to_network(uint8_t v) {
        return v;
    }
    inline int8_t to_network(int8_t v) {
        return v;
    }
    inline int8_t to_network(bool v) {
        return int8_t(v);
    }
    inline uint16_t to_network(uint16_t v) {
        return htons(v);
    }
    inline uint32_t to_network(uint32_t v) {
        return htonl(v);
    }
    inline uint64_t to_network(uint64_t v) {
#if defined(__APPLE__) || defined(__linux__)
        return htobe64(v); // available on BSD/Linux
#elif defined(_WIN32)
        return htonll(v); // available on MSVC
#else
        if constexpr (std::endian::native == std::endian::little) {

            return __builtin_bswap64(v);
        } else {
            return v;
        }
#endif
    }


    inline uint8_t from_network(uint8_t v) {
        return v;
    }
    inline int8_t from_network(int8_t v) {
        return v;
    }
    inline uint16_t from_network(uint16_t v) {
        return ntohs(v);
    }
    inline uint32_t from_network(uint32_t v) {
        return ntohl(v);
    }
    inline uint64_t from_network(uint64_t v) {
#if defined(__APPLE__) || defined(__linux__)
        return be64toh(v);
#elif defined(_WIN32)
        return ntohll(v);
#else
        if constexpr (std::endian::native == std::endian::little) {
            return __builtin_bswap64(v);
        } else {
            return v;
        }
#endif
    }

    inline int16_t to_network(int16_t v) {
        uint16_t u;
        std::memcpy(&u, &v, sizeof(u));
        u = htons(u);
        int16_t out;
        std::memcpy(&out, &u, sizeof(out));
        return out;
    }

    inline int16_t from_network(int16_t v) {
        uint16_t u;
        std::memcpy(&u, &v, sizeof(u));
        u = ntohl(u);
        int16_t out;
        std::memcpy(&out, &u, sizeof(out));
        return out;
    }

    inline int32_t to_network(int32_t v) {
        uint32_t u;
        std::memcpy(&u, &v, sizeof(u));
        u = htonl(u);
        int32_t out;
        std::memcpy(&out, &u, sizeof(out));
        return out;
    }

    inline int32_t from_network(int32_t v) {
        uint32_t u;
        std::memcpy(&u, &v, sizeof(u));
        u = ntohl(u);
        int32_t out;
        std::memcpy(&out, &u, sizeof(out));
        return out;
    }

    inline uint64_t bswap64(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap64(x);
#elif defined(_MSC_VER)
        return _byteswap_uint64(x);
#else
        return ((x & 0x00000000000000FFULL) << 56) | ((x & 0x000000000000FF00ULL) << 40) | ((x & 0x0000000000FF0000ULL) << 24) |
               ((x & 0x00000000FF000000ULL) << 8) | ((x & 0x000000FF00000000ULL) >> 8) | ((x & 0x0000FF0000000000ULL) >> 24) |
               ((x & 0x00FF000000000000ULL) >> 40) | ((x & 0xFF00000000000000ULL) >> 56);
#endif
    }

    inline int64_t to_network(int64_t v) {
        uint64_t u;
        std::memcpy(&u, &v, sizeof(u));
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        u = bswap64(u);
#endif
        int64_t out;
        std::memcpy(&out, &u, sizeof(out));
        return out;
    }

    inline int64_t from_network(int64_t v) {
        uint64_t u;
        std::memcpy(&u, &v, sizeof(u));
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        u = bswap64(u);
#endif
        int64_t out;
        std::memcpy(&out, &u, sizeof(out));
        return out;
    }

    inline float to_network(float f) {
        static_assert(sizeof(float) == 4);
        uint32_t i;
        std::memcpy(&i, &f, sizeof(f));
        i = htonl(i);
        float out;

        std::memcpy(&out, &i, sizeof(out));
        return out;
    }

    inline double to_network(double d) {
        static_assert(sizeof(double) == 8);
        uint64_t i;
        std::memcpy(&i, &d, sizeof(d));
        i = to_network(i); // 64-bit version
        double out;
        std::memcpy(&out, &i, sizeof(out));
        return out;
    }
} // namespace cpx::sql::postgres::detail

#define SERIALIZE(...)      cpx::serde::Serialize<PGresult, __VA_ARGS__>
#define DESERIALIZE(...)    cpx::serde::Deserialize<PGresult, __VA_ARGS__>
#define SERIALIZABLE(...)   cpx::serde::is_serializable_v<PGresult, __VA_ARGS__>
#define DESERIALIZABLE(...) cpx::serde::is_deserializable_v<PGresult, __VA_ARGS__>

#define SERIALIZER_FIELDS std::vector<cpx::sql::postgres::Value> &doc;

#define DESERIALIZER_FIELDS                                                                                                      \
    PGresult *res;                                                                                                               \
    int       row;                                                                                                               \
    int       col;                                                                                                               \
    bool      is_binary;

template <typename T>
struct SERIALIZE(T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>) {
    SERIALIZER_FIELDS

    void from(T value) {
        Oid oid;
        if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, int8_t> || std::is_same_v<T, bool>) {
            oid = 16;
        } else if constexpr (std::is_same_v<T, uint16_t> || std::is_same_v<T, int16_t>) {
            oid = 21;
        } else if constexpr (std::is_same_v<T, uint32_t> || std::is_same_v<T, int32_t>) {
            oid = 23;
        } else if constexpr (std::is_same_v<T, uint64_t> || std::is_same_v<T, int64_t>) {
            oid = 20;
        } else if constexpr (std::is_same_v<T, float>) {
            oid = 700;
        } else if constexpr (std::is_same_v<T, double>) {
            oid = 701;
        }
        T *data = (T *)malloc(sizeof(T));
        *data   = cpx::sql::postgres::detail::to_network(value);
        doc.push_back({(const char *)data, sizeof(T), 1, oid, true});
    }
};

template <typename CT>
struct SERIALIZE(std::basic_string_view<char, CT>) {
    SERIALIZER_FIELDS

    void from(std::basic_string_view<char, CT> value, bool is_blob = false, bool own = false) const {
        if (own) {
            auto data = (char *)malloc(value.size());
            std::memcpy(data, value.data(), value.size());
            doc.push_back({data, (int)value.size(), is_blob ? 1 : 0, is_blob ? 17u : 0u, true});
        } else {
            doc.push_back({value.data(), (int)value.size(), is_blob ? 1 : 0, is_blob ? 17u : 0u, false});
        }
    }
};

template <typename CT, typename A>
struct SERIALIZE(std::basic_string<char, CT, A>) {
    SERIALIZER_FIELDS

    void from(const std::basic_string<char, CT, A> &value, bool is_blob = false) const {
        SERIALIZE(std::basic_string_view<char, CT>){doc}.from(value, is_blob, false);
    }

    void from(std::basic_string<char, CT, A> &&value, bool is_blob = false) const {
        SERIALIZE(std::basic_string_view<char, CT>){doc}.from(value, is_blob, true);
    }
};

template <typename A>
struct SERIALIZE(std::vector<uint8_t, A>) {
    SERIALIZER_FIELDS

    void from(const std::vector<uint8_t, A> &value) const {
        SERIALIZE(std::basic_string<char>){doc}.from(value, false);
    }

    void from(std::vector<uint8_t, A> &&value) const {
        SERIALIZE(std::basic_string<char>){doc}.from(value, true);
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
        SERIALIZE(decltype(t)){doc}.from(t);
    }
};

template <typename T>
struct SERIALIZE(std::optional<T>, std::enable_if_t<SERIALIZABLE(T)>) {
    SERIALIZER_FIELDS

    void from(const std::optional<T> &value) const {
        if (!value.has_value())
            doc.push_back({nullptr, 0, 0, 0, false});
        else
            SERIALIZE(T){doc}.from(*value);
    }

    void from(std::optional<T> &&value) const {
        if (!value.has_value())
            doc.push_back({nullptr, 0, 0, 0, false});
        else
            SERIALIZE(T){doc}.from(std::move(*value));
    }
};

template <typename T>
struct SERIALIZE(std::vector<T>, std::enable_if_t<SERIALIZABLE(T) && !std::is_same_v<T, uint8_t>>) {
    SERIALIZER_FIELDS

    void from(const std::vector<T> &value) const {
        for (auto &v : value)
            SERIALIZE(T){doc}.from(v);
    }

    void from(std::optional<T> &&value) const {
        for (auto &v : value)
            SERIALIZE(T){doc}.from(std::move(v));
    }
};

template <typename... Ts>
struct SERIALIZE(std::tuple<Ts...>, std::enable_if_t<(SERIALIZABLE(Ts) && ...)>) {
    SERIALIZER_FIELDS

    void from(const std::tuple<Ts...> &value) const {
        std::apply([&](auto &&...args) { (SERIALIZE(Ts){doc}.from(args), ...); }, value);
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<(std::is_integral_v<T> || std::is_floating_point_v<T>) && !std::is_same_v<T, bool>>) {
    DESERIALIZER_FIELDS

    void into(T &value) const {
        if (auto ptr = PQgetvalue(res, row, col); is_binary) {
            value = cpx::sql::postgres::detail::from_network(*reinterpret_cast<T *>(ptr));
        } else {
            value = (T)std::stoi(ptr);
        }
    }
};

template <>
struct DESERIALIZE(bool) {
    DESERIALIZER_FIELDS

    void into(bool &value) const {
        if (auto ptr = PQgetvalue(res, row, col); is_binary) {
            value = *ptr;
        } else {
            std::string v = ptr;
            value         = (v == "t" || v == "true" || v == "1");
        }
    }
};

template <typename CT, typename T>
struct DESERIALIZE(std::basic_string<char, CT, T>) {
    DESERIALIZER_FIELDS

    void into(std::basic_string<char, CT, T> &value) const {
        value = PQgetvalue(res, row, col);
    }
};

template <typename A>
struct DESERIALIZE(std::vector<uint8_t, A>) {
    DESERIALIZER_FIELDS

    void into(std::vector<uint8_t, A> &value) const {
        if (auto ptr = PQgetvalue(res, row, col); is_binary) {
            auto end = ptr + PQgetlength(res, row, col);
            value    = {ptr, end};
        } else {
            throw error("TODO: text format for bytea is not supported yet");
        }
    }
};

template <>
struct DESERIALIZE(std::tm) {
    DESERIALIZER_FIELDS

    void into(std::tm &value) const {
        std::time_t t;
        DESERIALIZE(std::time_t){res, row, col, is_binary}.into(t);
        value = *std::gmtime(&t);
    }
};

template <typename T>
struct DESERIALIZE(std::optional<T>, std::enable_if_t<DESERIALIZABLE(T)>) {
    DESERIALIZER_FIELDS

    void into(std::optional<T> &value) const {
        if (PQgetisnull(res, row, col)) {
            value = std::nullopt;
        } else {
            value.emplace();
            DESERIALIZE(T){res, row, col, is_binary}.into(*value);
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
