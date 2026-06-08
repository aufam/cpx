#ifndef CPX_SQL_MYSQL_H
#define CPX_SQL_MYSQL_H

#include <cpx/sql/sql.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/serde/error.h>
#include <cpx/defer.h>
#include <cstring>
#include <mysql/mysql.h>

namespace cpx::sql::mysql {
    using Value = MYSQL_BIND;

    class Connection;

    template <typename Row>
    class Rows;

    template <typename From, typename Enable = void>
    using Serialize = ::cpx::serde::Serialize<Value, From, Enable>;

    template <typename To, typename Enable = void>
    using Deserialize = ::cpx::serde::Deserialize<Value, To, Enable>;
} // namespace cpx::sql::mysql

namespace cpx::sql::mysql {
    template <typename Row>
    class Rows : public cpx::sql::Rows<Row> {
    public:
        explicit Rows(MYSQL_STMT *stmt)
            : stmt(stmt) {
            try {
                bind_result();
                next();
            } catch (std::runtime_error &e) {
                mysql_stmt_free_result(stmt);
                mysql_stmt_close(stmt);
                throw;
            }
        }

        virtual ~Rows() {
            mysql_stmt_free_result(stmt);
            mysql_stmt_close(stmt);
        }

        void next() {
            if (result_binds.empty()) {
                done = true;
                return;
            }

            if (int ret = mysql_stmt_fetch(stmt); ret == 0) {
                done = false;
            } else if (ret == MYSQL_NO_DATA) {
                done = true;
            } else if (ret == MYSQL_DATA_TRUNCATED) {
                fetch_result(std::make_index_sequence<std::tuple_size_v<Row>>());
                done = false;
            } else {
                unsigned int err  = mysql_stmt_errno(stmt);
                std::string  what = mysql_stmt_error(stmt);
                std::string  msg  = "Failed to fetch: " + std::to_string(err) + ": " + std::to_string(ret) + ": " + what;
                throw std::runtime_error(msg);
            }
        }

        Row get() const override {
            Row row = {};
            cpx::tuple_for_each(row, [&](auto &item, size_t i) {
                Deserialize<std::decay_t<decltype(item)>>{result_binds[i]}.into(item);
            });
            return row;
        }

        bool is_done() const override {
            return done;
        }

    protected:
        void bind_result() {
            prepare_result(std::make_index_sequence<std::tuple_size_v<Row>>());

            if (!result_binds.empty() && mysql_stmt_bind_result(stmt, result_binds.data())) {
                unsigned int err  = mysql_stmt_errno(stmt);
                std::string  what = mysql_stmt_error(stmt);
                std::string  msg  = "Bind result failed: " + std::to_string(err) + ": " + what;
                throw std::runtime_error(msg);
            }
        }

        template <size_t... I>
        void prepare_result(std::index_sequence<I...>) {
            (prepare_result_for<I>(), ...);
        }

        template <size_t i>
        void prepare_result_for() {
            using T = std::tuple_element_t<i, Row>;

            MYSQL_BIND &bind = result_binds[i] = {};

            bind.buffer        = &storage.at(i);
            bind.buffer_length = 8;
            bind.is_unsigned   = std::is_unsigned_v<T>;
            bind.length        = &lengths[i];
            bind.is_null       = &nulls[i];
            bind.buffer_type   = mysql_type<T>(); // TODO
        }

        template <size_t... I>
        void fetch_result(std::index_sequence<I...>) {
            (fetch_result_for<I>(), ...);
        }

        template <size_t i>
        void fetch_result_for() {
            using T = std::tuple_element_t<i, Row>;

            MYSQL_BIND &bind = result_binds[i];

            if (*bind.length <= bind.buffer_length)
                return; // no need to fetch

            std::vector<char> &vec = dynamic_storage[i];
            vec.resize(*bind.length);
            bind.buffer        = vec.data();
            bind.buffer_length = vec.size();

            if (!mysql_stmt_fetch_column(stmt, &bind, i, 0)) {
                std::string  what = mysql_error(mysql);
                unsigned int err  = mysql_errno(mysql);
                std::string  msg  = "Cannot connect: " + std::to_string(err) + ": " + what;
                throw std::runtime_error(msg);
            }
        }

        static constexpr auto size = std::tuple_size_v<Row>;

        MYSQL      *mysql;
        MYSQL_STMT *stmt;
        bool        done = false;

        std::array<MYSQL_BIND, size>        result_binds    = {};
        std::array<uint64_t, size>          storage         = {};
        std::array<size_t, size>            lengths         = {};
        std::array<char, size>              nulls           = {};
        std::array<std::vector<char>, size> dynamic_storage = {};
    };

    class Connection : public cpx::sql::Connection {
    public:
        Connection(
            const std::string &host,
            const std::string &user,
            const std::string &pass,
            const std::string &db,
            unsigned int       port = 3306
        ) {
            mysql = mysql_init(nullptr);
            if (!mysql)
                throw std::runtime_error("mysql_init() failed");

            if (!mysql_real_connect(mysql, host.c_str(), user.c_str(), pass.c_str(), db.c_str(), port, nullptr, 0)) {
                std::string  what = mysql_error(mysql);
                unsigned int err  = mysql_errno(mysql);
                std::string  msg  = "Cannot connect: " + std::to_string(err) + ": " + what;
                mysql_close(mysql);
                throw std::runtime_error(msg);
            }
        }

        ~Connection() override {
            mysql_close(mysql);
        }

        template <typename Params, typename Row>
        Rows<Row> operator()(const Statement<Params, Row> &statement) {
            MYSQL_STMT *stmt = mysql_stmt_init(mysql);
            if (!stmt)
                throw std::runtime_error("mysql_stmt_init() failed");

            if (mysql_stmt_prepare(stmt, statement.query.c_str(), statement.query.size())) {
                std::string  what = mysql_error(mysql);
                unsigned int err  = mysql_errno(mysql);
                std::string  msg  = "Failed to prepare statement `" + statement.query + "`: " + std::to_string(err) + ": " + what;
                throw std::runtime_error(msg);
            }

            std::vector<MYSQL_BIND> doc;
            std::vector<uint64_t>   storage;
            doc.reserve(std::tuple_size_v<Params>);
            storage.reserve(std::tuple_size_v<Params>);
            Serialize<Params>{doc, storage}.from(statement.params);

            if (!doc.empty() && mysql_stmt_bind_param(stmt, doc.data())) {
                unsigned int err  = mysql_stmt_errno(stmt);
                std::string  what = mysql_stmt_error(stmt);
                std::string  msg  = "Bind param failed: " + std::to_string(err) + ": " + what;
                throw std::runtime_error(msg);
            }

            if (mysql_stmt_execute(stmt)) {
                unsigned int err  = mysql_stmt_errno(stmt);
                std::string  what = mysql_stmt_error(stmt);
                std::string  msg  = "Execute failed: " + std::to_string(err) + ": " + what;
                throw std::runtime_error(msg);
            }

            return Rows<Row>(stmt);
        }

    protected:
        MYSQL *mysql;
    };
} // namespace cpx::sql::mysql

namespace cpx::sql::mysql::detail {
    inline std::string to_string(enum_field_types type) {
        switch (type) {
        case MYSQL_TYPE_DECIMAL:
            return "decimal";

        case MYSQL_TYPE_TINY:
            return "tiny";

        case MYSQL_TYPE_SHORT:
            return "short";

        case MYSQL_TYPE_LONG:
            return "long";

        case MYSQL_TYPE_FLOAT:
            return "float";

        case MYSQL_TYPE_DOUBLE:
            return "double";

        case MYSQL_TYPE_NULL:
            return "null";

        case MYSQL_TYPE_TIMESTAMP:
            return "timestamp";

        case MYSQL_TYPE_LONGLONG:
            return "longlong";

        case MYSQL_TYPE_INT24:
            return "int24";

        case MYSQL_TYPE_DATE:
            return "date";

        case MYSQL_TYPE_TIME:
            return "time";

        case MYSQL_TYPE_DATETIME:
            return "datetime";

        case MYSQL_TYPE_YEAR:
            return "year";

        case MYSQL_TYPE_NEWDATE:
            return "newdate";

        case MYSQL_TYPE_VARCHAR:
            return "varchar";

        case MYSQL_TYPE_BIT:
            return "bit";

        case MYSQL_TYPE_JSON:
            return "json";

        case MYSQL_TYPE_NEWDECIMAL:
            return "newdecimal";

        case MYSQL_TYPE_ENUM:
            return "enum";

        case MYSQL_TYPE_SET:
            return "set";

        case MYSQL_TYPE_TINY_BLOB:
            return "tiny blob";

        case MYSQL_TYPE_MEDIUM_BLOB:
            return "medium blob";

        case MYSQL_TYPE_LONG_BLOB:
            return "long blob";

        case MYSQL_TYPE_BLOB:
            return "blob";

        case MYSQL_TYPE_VAR_STRING:
            return "var string";

        case MYSQL_TYPE_STRING:
            return "string";

        case MYSQL_TYPE_GEOMETRY:
            return "geometry";

#ifdef MYSQL_TYPE_BOOL
        case MYSQL_TYPE_BOOL:
            return "bool";
#endif

        default:
            return "unknown";
        }
    }
} // namespace cpx::sql::mysql::detail

#define SERIALIZE(...)      cpx::serde::Serialize<MYSQL_BIND, __VA_ARGS__>
#define DESERIALIZE(...)    cpx::serde::Deserialize<MYSQL_BIND, __VA_ARGS__>
#define SERIALIZABLE(...)   cpx::serde::is_serializable_v<MYSQL_BIND, __VA_ARGS__>
#define DESERIALIZABLE(...) cpx::serde::is_deserializable_v<MYSQL_BIND, __VA_ARGS__>

#define SERIALIZER_FIELDS std::vector<MYSQL_BIND> &doc;

template <typename T>
struct SERIALIZE(T, std::enable_if_t<std::is_arithmetic_v<T>>) {
    std::vector<MYSQL_BIND> &doc;
    std::vector<uint64_t>   &storage;

    void from(T value) {
        storage.emplace_back();
        std::memcpy(&storage.back(), &value, sizeof(T));

        MYSQL_BIND bind{};
        bind.buffer        = &storage.back();
        bind.buffer_length = sizeof(T);
        bind.is_unsigned   = std::is_unsigned_v<T>;

        if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t>) {
            bind.buffer_type = MYSQL_TYPE_TINY;
        } else if constexpr (sizeof(T) == 2 && std::is_integral_v<T>) {
            bind.buffer_type = MYSQL_TYPE_SHORT;
        } else if constexpr (sizeof(T) == 4 && std::is_integral_v<T>) {
            bind.buffer_type = MYSQL_TYPE_LONG;
        } else if constexpr (sizeof(T) == 8 && std::is_integral_v<T>) {
            bind.buffer_type = MYSQL_TYPE_LONGLONG;
        } else if constexpr (std::is_same_v<T, float>) {
            bind.buffer_type = MYSQL_TYPE_FLOAT;
        } else if constexpr (std::is_same_v<T, double>) {
            bind.buffer_type = MYSQL_TYPE_DOUBLE;
        }

        doc.push_back(bind);
    }
};

template <typename CT>
struct SERIALIZE(std::basic_string_view<char, CT>) {
    std::vector<MYSQL_BIND>          &doc;
    std::vector<std::array<char, 8>> &storage;

    void from(std::basic_string_view<char, CT> value, bool is_blob = false) {
        MYSQL_BIND bind{};
        bind.buffer        = value.data();
        bind.buffer_length = value.size();
        bind.buffer_type   = is_blob ? MYSQL_TYPE_BLOB : MYSQL_TYPE_STRING;
        doc.push_back(bind);
    }
};

template <typename CT, typename A>
struct SERIALIZE(std::basic_string<char, CT, A>) {
    std::vector<MYSQL_BIND>          &doc;
    std::vector<std::array<char, 8>> &storage;

    void from(const std::basic_string<char, CT, A> &value, bool is_blob = false) const {
        SERIALIZE(std::basic_string_view<char, CT>){doc, storage}.from(value, is_blob);
    }
};

template <typename A>
struct SERIALIZE(std::vector<uint8_t, A>) {
    std::vector<MYSQL_BIND>          &doc;
    std::vector<std::array<char, 8>> &storage;

    void from(const std::vector<uint8_t, A> &value) const {
        SERIALIZE(std::string_view){doc, storage}.from(
            std::string_view(reinterpret_cast<const char *>(value.data()), value.size()), true
        );
    }
};

template <>
struct SERIALIZE(std::tm) {
    std::vector<MYSQL_BIND>          &doc;
    std::vector<std::array<char, 8>> &storage;

    void from(std::tm value) const {
#if defined(_WIN32)
        auto t = _mkgmtime(&value);
#else
        auto t = ::timegm(&value);
#endif
        SERIALIZE(decltype(t)){doc, storage}.from(t);
    }
};

template <typename T>
struct SERIALIZE(std::optional<T>, std::enable_if_t<SERIALIZABLE(T)>) {
    std::vector<MYSQL_BIND>          &doc;
    std::vector<std::array<char, 8>> &storage;

    void from(const std::optional<T> &value) const {
        if (!value.has_value()) {
            storage.emplace_back();
            auto &nil = storage.back()[0];
            nil       = 1;

            MYSQL_BIND bind{};
            bind.is_null     = &nil;
            bind.buffer_type = MYSQL_TYPE_NULL;
        } else
            SERIALIZE(T){doc, storage}.from(*value);
    }
};

template <typename T>
struct SERIALIZE(std::vector<T>, std::enable_if_t<SERIALIZABLE(T) && !std::is_same_v<T, uint8_t>>) {
    std::vector<MYSQL_BIND>          &doc;
    std::vector<std::array<char, 8>> &storage;

    void from(const std::vector<T> &value) const {
        for (auto &v : value)
            SERIALIZE(T){doc, storage}.from(v);
    }
};

template <typename... Ts>
struct SERIALIZE(std::tuple<Ts...>, std::enable_if_t<(SERIALIZABLE(Ts) && ...)>) {
    std::vector<MYSQL_BIND>          &doc;
    std::vector<std::array<char, 8>> &storage;

    void from(const std::tuple<Ts...> &value) const {
        std::apply([&](auto &&...args) { (SERIALIZE(Ts){doc, storage}.from(args), ...); }, value);
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<std::is_arithmetic_v<T>>) {
    MYSQL_BIND const &bind;

    void into(T &value) {
        if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t>) {
            if (bind.buffer_type != MYSQL_TYPE_TINY)
                throw type_mismatch_error("tiny", cpx::sql::mysql::detail::to_string(bind.buffer_type));
        } else if constexpr (sizeof(T) == 2 && std::is_integral_v<T>) {
            if (bind.buffer_type != MYSQL_TYPE_SHORT)
                throw type_mismatch_error("short", cpx::sql::mysql::detail::to_string(bind.buffer_type));
        } else if constexpr (sizeof(T) == 4 && std::is_integral_v<T>) {
            if (bind.buffer_type != MYSQL_TYPE_LONG)
                throw type_mismatch_error("long", cpx::sql::mysql::detail::to_string(bind.buffer_type));
        } else if constexpr (sizeof(T) == 8 && std::is_integral_v<T>) {
            if (bind.buffer_type != MYSQL_TYPE_LONGLONG)
                throw type_mismatch_error("longlong", cpx::sql::mysql::detail::to_string(bind.buffer_type));
        } else if constexpr (std::is_same_v<T, float>) {
            if (bind.buffer_type != MYSQL_TYPE_FLOAT)
                throw type_mismatch_error("float", cpx::sql::mysql::detail::to_string(bind.buffer_type));
        } else if constexpr (std::is_same_v<T, double>) {
            if (bind.buffer_type != MYSQL_TYPE_DOUBLE)
                throw type_mismatch_error("double", cpx::sql::mysql::detail::to_string(bind.buffer_type));
        }

        std::memcpy(&value, bind.buffer, sizeof(T));
    }
};

template <typename CT>
struct DESERIALIZE(std::basic_string_view<char, CT>) {
    MYSQL_BIND const &bind;

    void into(std::basic_string_view<char, CT> &value) const {
        const auto t = bind.buffer_type;

        if (t != MYSQL_TYPE_STRING && t != MYSQL_TYPE_VAR_STRING && t != MYSQL_TYPE_BLOB && t != MYSQL_TYPE_TINY_BLOB &&
            t != MYSQL_TYPE_MEDIUM_BLOB && t != MYSQL_TYPE_LONG_BLOB) {
            throw type_mismatch_error("string", cpx::sql::mysql::detail::to_string(t));
        }

        auto size = bind.length ? static_cast<size_t>(*bind.length) : static_cast<size_t>(bind.buffer_length);
        value     = {static_cast<const char *>(bind.buffer), size};
    }
};

template <typename CT, typename A>
struct DESERIALIZE(std::basic_string<char, CT, A>) {
    MYSQL_BIND const &bind;

    void into(std::basic_string<char, CT, A> &value) const {
        std::basic_string_view<char, CT> str;
        DESERIALIZE(decltype(str)){bind}.into(str);
        value = std::basic_string<char, CT, A>(str);
    }
};

template <>
struct DESERIALIZE(std::tm) {
    MYSQL_BIND const &bind;

    void into(std::tm &value) const {
        std::time_t t;
        DESERIALIZE(std::time_t){bind}.into(t);
        value = *std::gmtime(&t);
    }
};

template <typename A>
struct DESERIALIZE(std::vector<uint8_t, A>) {
    MYSQL_BIND const &bind;

    void into(std::vector<uint8_t> &value) const {
        const auto t = bind.buffer_type;

        if (t != MYSQL_TYPE_BLOB && t != MYSQL_TYPE_TINY_BLOB && t != MYSQL_TYPE_MEDIUM_BLOB && t != MYSQL_TYPE_LONG_BLOB) {
            throw type_mismatch_error("string", cpx::sql::mysql::detail::to_string(t));
        }

        auto ptr  = static_cast<const uint8_t *>(bind.buffer);
        auto size = bind.length ? static_cast<size_t>(*bind.length) : static_cast<size_t>(bind.buffer_length);
        value     = {ptr, ptr + size};
    }
};

template <typename T>
struct DESERIALIZE(std::optional<T>, std::enable_if_t<DESERIALIZABLE(T)>) {
    MYSQL_BIND const &bind;

    void into(std::optional<T> &value) const {
        if (bind.buffer_type != MYSQL_TYPE_NULL) {
            value = std::nullopt;
        } else {
            value.emplace();
            DESERIALIZE(T){bind}.into(*value);
        }
    }
};
#endif
