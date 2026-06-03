#ifndef CPX_SQL_SQLITE3_H
#define CPX_SQL_SQLITE3_H

#include <cpx/sql/sql.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/serde/error.h>
#include <string>
#include <optional>
#include <ctime>
#include <utility>

#ifndef SQLITE3_H
#    include <sqlite3.h>
#endif


/*
 * Forward declarations
 */
namespace cpx::sql::sqlite3 {
    class Connection;

    template <typename Row>
    class Rows;

    template <typename From, typename Enable = void>
    using Serialize = cpx::serde::Serialize<sqlite3_stmt, From, Enable>;

    template <typename To, typename Enable = void>
    using Deserialize = cpx::serde::Deserialize<sqlite3_stmt, To, Enable>;

    class error : public std::exception {
        mutable std::string what_;

    public:
        std::string title, content, query;
        int         ret;

        explicit error(std::string title, std::string content = "", std::string query = "", int ret = SQLITE_OK)
            : title(std::move(title))
            , content(std::move(content))
            , query(std::move(query))
            , ret(ret) {}

        const char *what() const noexcept override {
            what_ = title;
            if (!content.empty()) {
                what_ += ": ";
                what_ += content;
            }
            bool has_query = false;
            if (!query.empty()) {
                has_query = true;
                what_ += ": query=\"";
                what_ += query;
                what_ += '\"';
            }
            if (ret != SQLITE_OK) {
                what_ += has_query ? " ret=" : ": ret=";
                what_ += std::to_string(ret);
            }
            return what_.c_str();
        }
    };
} // namespace cpx::sql::sqlite3

/*
 * Implementations
 */
namespace cpx::sql::sqlite3 {
    template <typename Row>
    class Rows : public cpx::sql::Rows<Row> {
        friend class Connection;

    protected:
        Rows(struct sqlite3 *db, sqlite3_stmt *stmt, std::string query)
            : db(db)
            , stmt(stmt)
            , query(std::move(query)) {
            try {
                next();
            } catch (...) {
                sqlite3_finalize(stmt);
                throw;
            }
        }

    public:
        Rows(const Rows &) = delete;

        Rows(Rows &&other) noexcept
            : db(other.db)
            , stmt(std::exchange(other.stmt, nullptr))
            , query(std::move(other.query))
            , ret(std::exchange(other.ret, SQLITE_DONE)) {}

        ~Rows() override {
            if (stmt) {
                sqlite3_finalize(stmt);
                stmt = nullptr;
            }
        }

        void next() override {
            ret = sqlite3_step(stmt);
            if (ret != SQLITE_DONE && ret != SQLITE_ROW)
                throw error("Failed to step", sqlite3_errmsg(db), query, ret);
        }

        Row get() const override {
            if (ret != SQLITE_ROW)
                throw error("Failed to step", "not a row", query, ret);

            return get_all(std::make_index_sequence<std::tuple_size_v<Row>>());
        }

        bool is_done() const override {
            return ret == SQLITE_DONE;
        }

        sqlite3_int64 get_last_insert_rowid() const {
            return sqlite3_last_insert_rowid(db);
        }

    protected:
        struct sqlite3 *db;
        sqlite3_stmt   *stmt;
        std::string     query;
        int             ret;

        template <std::size_t... I>
        auto get_all(std::index_sequence<I...>) const {
            Row row = {};
            (Deserialize<std::tuple_element_t<I, Row>>{stmt, int(I)}.into(std::get<I>(row)), ...);
            return row;
        }
    };

    class Connection : public cpx::sql::Connection {
    public:
        Connection(const Connection &) = delete;

        Connection(Connection &&other) noexcept
            : db(std::exchange(other.db, nullptr))
            , stmt(std::exchange(other.stmt, nullptr)) {}

        Connection(const std::string &filename) {
            int ret = sqlite3_open(filename.c_str(), &db);
            if (ret != SQLITE_OK) {
                std::string content = sqlite3_errmsg(db);
                sqlite3_close(db);
                throw error("Cannot open \"" + filename + "\"", content, "", ret);
            }
        }

        void begin_transaction() override {
            char *errmsg = nullptr;
            int   ret    = sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, &errmsg);
            if (ret != SQLITE_OK) {
                std::string content = errmsg ? errmsg : "";
                sqlite3_free(errmsg);
                throw error("Failed to begin transaction", content, "BEGIN TRANSACTION", ret);
            }
        }

        void commit() override {
            char *errmsg = nullptr;
            int   ret    = sqlite3_exec(db, "COMMIT", nullptr, nullptr, &errmsg);
            if (ret != SQLITE_OK) {
                std::string content = errmsg ? errmsg : "";
                sqlite3_free(errmsg);
                throw error("Failed to commit transaction", content, "COMMIT", ret);
            }
        }

        void cancel() override {
            char *errmsg = nullptr;
            int   ret    = sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, &errmsg);
            if (ret != SQLITE_OK) {
                std::string content = errmsg ? errmsg : "";
                sqlite3_free(errmsg);
                throw error("Failed to cancel transaction", content, "ROLLBACK", ret);
            }
        }

        template <typename Params, typename Row>
        Rows<Row> operator()(Statement<Params, Row> statement) {
            int ret = sqlite3_prepare_v2(db, statement.query.c_str(), -1, &stmt, nullptr);
            if (ret != SQLITE_OK) {
                std::string content = sqlite3_errmsg(db);
                throw error("Failed to prepare statement", std::move(content), std::move(statement.query), ret);
            }

            std::apply(
                [&](auto &&...args) {
                    [[maybe_unused]]
                    int index = 1;
                    (Serialize<std::decay_t<decltype(args)>>{stmt, index}.from(args), ...);
                },
                std::move(statement.params)
            );

            return {db, stmt, std::move(statement.query)};
        }

        ~Connection() override {
            sqlite3_close(db);
        }

    protected:
        struct sqlite3      *db;
        struct sqlite3_stmt *stmt;
    };
} // namespace cpx::sql::sqlite3


#define SERIALIZE(...)      cpx::serde::Serialize<sqlite3_stmt, __VA_ARGS__>
#define DESERIALIZE(...)    cpx::serde::Deserialize<sqlite3_stmt, __VA_ARGS__>
#define SERIALIZABLE(...)   cpx::serde::is_serializable_v<sqlite3_stmt, __VA_ARGS__>
#define DESERIALIZABLE(...) cpx::serde::is_deserializable_v<sqlite3_stmt, __VA_ARGS__>

#define SERIALIZER_FIELDS                                                                                                        \
    sqlite3_stmt *stmt;                                                                                                          \
    int          &index

#define DESERIALIZER_FIELDS                                                                                                      \
    sqlite3_stmt *stmt;                                                                                                          \
    int           index

template <typename T>
struct SERIALIZE(T, std::enable_if_t<std::is_integral_v<T>>) {
    SERIALIZER_FIELDS;
    void from(T value) const {
        if constexpr (sizeof(T) <= 4)
            sqlite3_bind_int(stmt, index++, int(value));
        else
            sqlite3_bind_int64(stmt, index++, sqlite3_int64(value));
    }
};

template <typename T>
struct SERIALIZE(T, std::enable_if_t<std::is_floating_point_v<T>>) {
    SERIALIZER_FIELDS;
    void from(T value) const {
        sqlite3_bind_double(stmt, index++, double(value));
    }
};

template <>
struct SERIALIZE(std::tm) {
    SERIALIZER_FIELDS;
    void from(std::tm value) const {
#if defined(_WIN32)
        auto t = _mkgmtime(&value);
#else
        auto t = ::timegm(&value);
#endif
        sqlite3_bind_int64(stmt, index++, t);
    }
};

template <typename CT>
struct SERIALIZE(std::basic_string_view<char, CT>) {
    SERIALIZER_FIELDS;
    void from(std::basic_string_view<char, CT> value, bool is_blob = false) const {
        if (is_blob) {
            sqlite3_bind_blob(stmt, index++, (void *)value.data(), (int)value.size(), SQLITE_STATIC);
        } else {
            sqlite3_bind_text(stmt, index++, value.data(), (int)value.size(), SQLITE_STATIC);
        }
    }
};

template <typename CT, typename A>
struct SERIALIZE(std::basic_string<char, CT, A>) {
    SERIALIZER_FIELDS;
    void from(const std::basic_string<char, CT, A> &value, bool is_blob = false) const {
        return SERIALIZE(std::basic_string_view<char, CT>){stmt, index}.from(value, is_blob);
    }
    void from(std::basic_string<char, CT, A> &&value) = delete;
};

template <typename T>
struct SERIALIZE(std::optional<T>, std::enable_if_t<SERIALIZABLE(T)>) {
    SERIALIZER_FIELDS;
    void from(const std::optional<T> &value) const {
        if (!value.has_value())
            sqlite3_bind_null(stmt, index++);
        else
            SERIALIZE(T){stmt, index}.from(*value);
    }
    void from(std::vector<T> &&) = delete;
};

template <typename T>
struct SERIALIZE(std::vector<T>, std::enable_if_t<SERIALIZABLE(T)>) {
    SERIALIZER_FIELDS;
    void from(const std::vector<T> &value) const {
        if constexpr (std::is_same_v<T, uint8_t>) {
            sqlite3_bind_blob(stmt, index++, (void *)value.data(), (int)value.size(), SQLITE_STATIC);
        } else {
            for (auto &val : value)
                SERIALIZE(T){stmt, index}.from(val);
        }
    }
    void from(std::vector<T> &&) = delete;
};

template <typename... Ts>
struct SERIALIZE(std::tuple<Ts...>, std::enable_if_t<(SERIALIZABLE(Ts) && ...)>) {
    SERIALIZER_FIELDS;
    void from(const std::tuple<Ts...> &value) const {
        std::apply([&](auto &&...args) { (SERIALIZE(Ts){stmt, index}.from(args), ...); }, value);
    }
};


namespace cpx::sql::sqlite3::detail {
    inline std::string type_of(int type) {
        switch (type) {
        case SQLITE_INTEGER:
            return "integer";
        case SQLITE_FLOAT:
            return "float";
        case SQLITE_TEXT:
            return "text";
        case SQLITE_BLOB:
            return "blob";
        case SQLITE_NULL:
            return "null";
        default:
            return "unknown";
        }
    }
} // namespace cpx::sql::sqlite3::detail

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<std::is_integral_v<T>>) {
    DESERIALIZER_FIELDS;
    void into(T &value) const {
        if (auto type = sqlite3_column_type(stmt, index); type != SQLITE_INTEGER)
            throw type_mismatch_error("integer", cpx::sql::sqlite3::detail::type_of(type));
        if constexpr (sizeof(T) <= 4)
            value = (T)sqlite3_column_int(stmt, index);
        else
            value = (T)sqlite3_column_int64(stmt, index);
    }
};

template <>
struct DESERIALIZE(std::tm) {
    DESERIALIZER_FIELDS;
    void into(std::tm &value) const {
        if (auto type = sqlite3_column_type(stmt, index); type != SQLITE_INTEGER)
            throw type_mismatch_error("integer", cpx::sql::sqlite3::detail::type_of(type));
        std::time_t t;
        DESERIALIZE(std::time_t){stmt, index}.into(t);
        value = *std::gmtime(&t);
    }
};

template <typename T>
struct DESERIALIZE(T, std::enable_if_t<std::is_floating_point_v<T>>) {
    DESERIALIZER_FIELDS;
    void into(T &value) const {
        if (auto type = sqlite3_column_type(stmt, index); type != SQLITE_FLOAT)
            throw type_mismatch_error("float", cpx::sql::sqlite3::detail::type_of(type));
        value = (T)sqlite3_column_double(stmt, index);
    }
};

template <typename CT, typename A>
struct DESERIALIZE(std::basic_string<char, CT, A>) {
    DESERIALIZER_FIELDS;
    void into(std::basic_string<char, CT, A> &value) const {
        if (auto type = sqlite3_column_type(stmt, index); type != SQLITE_TEXT)
            throw type_mismatch_error("text", cpx::sql::sqlite3::detail::type_of(type));
        value = (const char *)sqlite3_column_text(stmt, index);
    }
};

template <>
struct DESERIALIZE(std::vector<uint8_t>) {
    DESERIALIZER_FIELDS;
    void into(std::vector<uint8_t> &value) const {
        if (auto type = sqlite3_column_type(stmt, index); type != SQLITE_BLOB)
            throw type_mismatch_error("blob", cpx::sql::sqlite3::detail::type_of(type));
        const auto *data = static_cast<const uint8_t *>(sqlite3_column_blob(stmt, index));
        const int   size = sqlite3_column_bytes(stmt, index);
        if (!data || size <= 0)
            return;
        value = {data, data + size};
    }
};

template <typename T>
struct DESERIALIZE(std::optional<T>, std::enable_if_t<DESERIALIZABLE(T)>) {
    DESERIALIZER_FIELDS;
    void into(std::optional<T> &value) const {
        if (sqlite3_column_type(stmt, index) == SQLITE_NULL) {
            value = std::nullopt;
        } else {
            value.emplace();
            DESERIALIZE(T){stmt, index}.into(*value);
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
