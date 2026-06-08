// TODO: perfect forwarding
// TODO: tuple of references

#ifndef CPX_SQL_SQL_H
#define CPX_SQL_SQL_H

#include <cpx/tuple.h>
#include <optional>
#include <string>
#include <vector>
#include <utility>

#ifndef BOOST_PFR_HPP
#    if __has_include(<boost/pfr.hpp>)
#        include <boost/pfr.hpp>
#    endif
#endif


/*
 * Forward declarations
 */
namespace cpx::sql {
    class Connection;

    template <typename Row>
    class Rows;

    template <typename Params = std::tuple<>, typename Row = std::tuple<>>
    struct Statement;

    template <typename Table, typename T>
    class Column;

    template <typename T>
    class Alias;

    template <typename Params = std::tuple<>>
    struct Assignment;

    template <typename Params = std::tuple<>>
    struct Condition;

    template <typename Params = std::tuple<>>
    struct Arithmetics;
} // namespace cpx::sql

/*
 * helper detail
 */
namespace cpx::sql::detail {
    // TODO: implement generic?
    struct GenericStatement {};
    struct GenericRows {};

    struct NoTable {
        static constexpr const char *TableName = nullptr;
    };

    // column type
    template <typename T>
    struct column;

    template <typename Table, typename T>
    struct column<sql::Column<Table, T>> {
        using type = typename sql::Column<Table, T>::type;
    };

    // is_column
    template <typename T>
    struct is_column : std::false_type {};

    template <typename Table, typename T>
    struct is_column<Column<Table, T>> : std::true_type {};

    // repeated placeholders
    template <size_t i>
    struct repeated_placeholders {
        static std::string value() {
            return repeated_placeholders<i - 1>::value() + ", ?";
        }
    };

    template <>
    struct repeated_placeholders<0> {
        static std::string value() {
            return "";
        }
    };

    template <>
    struct repeated_placeholders<1> {
        static std::string value() {
            return "?";
        }
    };

    // string cat
    template <typename Sep>
    Sep string_cat(const Sep &) {
        return "";
    }

    template <typename Sep, typename Arg, typename... Args>
    auto string_cat(const Sep &separator, Arg &&arg, Args &&...args) {
        if constexpr (sizeof...(Args) > 0)
            return arg + ((separator + args) + ...);
        else
            return std::forward<Arg>(arg);
    }
} // namespace cpx::sql::detail


/*
 * Implementations
 */
namespace cpx::sql {
    class Connection {
    public:
        virtual ~Connection() = default;

        virtual void begin_transaction() = 0;
        virtual void commit()            = 0;
        virtual void cancel()            = 0;

        // TODO: implement generic?
        template <typename Params, typename Row>
        Rows<Row> operator()(const Statement<Params, Row> &statement);

    protected:
        // TODO: implement generic?
        virtual detail::GenericRows execute(const detail::GenericStatement &) {
            return {};
        };
    };

    template <typename Row>
    class Rows {
    public:
        virtual void next()          = 0;
        virtual bool is_done() const = 0;
        virtual Row  get() const     = 0;
        virtual ~Rows()              = default;

        template <typename F>
        void for_each(F &&fn) {
            for (; !is_done(); next())
                std::apply(fn, get());
        }
    };

    template <typename Params>
    struct Condition {
        using params_type = Params;
        std::string query;
        params_type params = {};

        template <typename OParams>
        auto operator&&(const Condition<OParams> &other) const {
            auto params = std::tuple_cat(this->params, other.params);
            return Condition<decltype(params)>{"(" + query + " and " + other.query + ")", std::move(params)};
        }

        template <typename OParams>
        auto operator||(const Condition<OParams> &other) const {
            auto params = std::tuple_cat(this->params, other.params);
            return Condition<decltype(params)>{"(" + query + " or " + other.query + ")", std::move(params)};
        }

        Condition<Params> operator!() const {
            return {"not (" + query + ")", params};
        }

        Statement<Params, std::tuple<>> into_statement() const {
            return {query, params};
        }

        // like
        Condition<Params> escape(char ch) const {
            return {query + " escape '" + ch + "'", params};
        }
    };

    template <typename Params>
    struct Assignment {
        using params_type = Params;
        std::string query;
        params_type params = {};

        template <typename OParams>
        auto operator,(const Assignment<OParams> &other) const {
            auto params = std::tuple_cat(this->params, other.params);
            return Assignment<decltype(params)>{query + ", " + other.query, std::move(params)};
        }

        Statement<Params, std::tuple<>> into_statement() const {
            return {query, params};
        }
    };

    template <typename Params>
    struct Arithmetics {
        using params_type = Params;
        std::string query;
        params_type params = {};

        Statement<Params, std::tuple<>> into_statement() const {
            return {query, params};
        }

        /*
         * with Self
         */
        template <typename OParams>
        auto operator+(Arithmetics<OParams> other) const {
            auto params = std::tuple_cat(this->params, other.params);
            return Assignment<decltype(params)>{"(" + query + " + " + other.query + ")", std::move(params)};
        }
        template <typename OParams>
        auto operator-(Arithmetics<OParams> other) const {
            auto params = std::tuple_cat(this->params, other.params);
            return Assignment<decltype(params)>{"(" + query + " - " + other.query + ")", std::move(params)};
        }
        template <typename OParams>
        auto operator*(Arithmetics<OParams> other) const {
            auto params = std::tuple_cat(this->params, other.params);
            return Assignment<decltype(params)>{"(" + query + " * " + other.query + ")", std::move(params)};
        }
        template <typename OParams>
        auto operator/(Arithmetics<OParams> other) const {
            auto params = std::tuple_cat(this->params, other.params);
            return Assignment<decltype(params)>{"(" + query + " / " + other.query + ")", std::move(params)};
        }
        template <typename OParams>
        auto operator==(Arithmetics<OParams> other) const {
            auto params = std::tuple_cat(this->params, other.params);
            return Condition<decltype(params)>{"(" + query + " = " + other.query + ")", std::move(params)};
        }
        template <typename OParams>
        auto operator!=(Arithmetics<OParams> other) const {
            auto params = std::tuple_cat(this->params, other.params);
            return Condition<decltype(params)>{"(" + query + " != " + other.query + ")", std::move(params)};
        }
        template <typename OParams>
        auto operator>(Arithmetics<OParams> other) const {
            auto params = std::tuple_cat(this->params, other.params);
            return Condition<decltype(params)>{"(" + query + " > " + other.query + ")", std::move(params)};
        }
        template <typename OParams>
        auto operator<(Arithmetics<OParams> other) const {
            auto params = std::tuple_cat(this->params, other.params);
            return Condition<decltype(params)>{"(" + query + " < " + other.query + ")", std::move(params)};
        }
        template <typename OParams>
        auto operator>=(Arithmetics<OParams> other) const {
            auto params = std::tuple_cat(this->params, other.params);
            return Condition<decltype(params)>{"(" + query + " >= " + other.query + ")", std::move(params)};
        }
        template <typename OParams>
        auto operator<=(Arithmetics<OParams> other) const {
            auto params = std::tuple_cat(this->params, other.params);
            return Condition<decltype(params)>{"(" + query + " <= " + other.query + ")", std::move(params)};
        }


        /*
         * with column
         */
        template <typename Tbl, typename U>
        Arithmetics<Params> operator+(const Column<Tbl, U> &col) const {
            return {"(" + query + " + " + col.qualified_name() + ")", params};
        }
        template <typename Tbl, typename U>
        Arithmetics<Params> operator-(const Column<Tbl, U> &col) const {
            return {"(" + query + " - " + col.qualified_name() + ")", params};
        }
        template <typename Tbl, typename U>
        Arithmetics<Params> operator*(const Column<Tbl, U> &col) const {
            return {"(" + query + " * " + col.qualified_name() + ")", params};
        }
        template <typename Tbl, typename U>
        Arithmetics<Params> operator/(const Column<Tbl, U> &col) const {
            return {"(" + query + " / " + col.qualified_name() + ")", params};
        }
        template <typename Tbl, typename U>
        Condition<Params> operator==(const Column<Tbl, U> &col) const {
            return {query + " = " + col.qualified_name(), params};
        }
        template <typename Tbl, typename U>
        Condition<Params> operator!=(const Column<Tbl, U> &col) const {
            return {query + " != " + col.qualified_name(), params};
        }
        template <typename Tbl, typename U>
        Condition<Params> operator>(const Column<Tbl, U> &col) const {
            return {query + " > " + col.qualified_name(), params};
        }
        template <typename Tbl, typename U>
        Condition<Params> operator<(const Column<Tbl, U> &col) const {
            return {query + " < " + col.qualified_name(), params};
        }
        template <typename Tbl, typename U>
        Condition<Params> operator>=(const Column<Tbl, U> &col) const {
            return {query + " >= " + col.qualified_name(), params};
        }
        template <typename Tbl, typename U>
        Condition<Params> operator<=(const Column<Tbl, U> &col) const {
            return {query + " <= " + col.qualified_name(), params};
        }


        /*
         * with string literal
         */
        template <size_t N>
        auto operator+(const char (&val)[N]) const {
            auto params = std::tuple_cat(this->params, std::tuple<std::string_view>{std::string_view(val, (-1))});
            return Arithmetics<decltype(params)>{"(" + query + " + ?)", std::move(params)};
        }
        template <size_t N>
        auto operator-(const char (&val)[N]) const {
            auto params = std::tuple_cat(this->params, std::tuple<std::string_view>{std::string_view(val, (-1))});
            return Arithmetics<decltype(params)>{"(" + query + " - ?)", std::move(params)};
        }
        template <size_t N>
        auto operator*(const char (&val)[N]) const {
            auto params = std::tuple_cat(this->params, std::tuple<std::string_view>{std::string_view(val, (-1))});
            return Arithmetics<decltype(params)>{"(" + query + " * ?)", std::move(params)};
        }
        template <size_t N>
        auto operator/(const char (&val)[N]) const {
            auto params = std::tuple_cat(this->params, std::tuple<std::string_view>{std::string_view(val, (-1))});
            return Arithmetics<decltype(params)>{"(" + query + " / ?)", std::move(params)};
        }
        template <size_t N>
        auto operator==(const char (&val)[N]) const {
            auto params = std::tuple_cat(this->params, std::tuple<std::string_view>{std::string_view(val, (-1))});
            return Condition<decltype(params)>{query + " = ?", std::move(params)};
        }
        template <size_t N>
        auto operator!=(const char (&val)[N]) const {
            auto params = std::tuple_cat(this->params, std::tuple<std::string_view>{std::string_view(val, (-1))});
            return Condition<decltype(params)>{query + " != ?", std::move(params)};
        }
        template <size_t N>
        auto operator>(const char (&val)[N]) const {
            auto params = std::tuple_cat(this->params, std::tuple<std::string_view>{std::string_view(val, (-1))});
            return Condition<decltype(params)>{query + " > ?", std::move(params)};
        }
        template <size_t N>
        auto operator<(const char (&val)[N]) const {
            auto params = std::tuple_cat(this->params, std::tuple<std::string_view>{std::string_view(val, (-1))});
            return Condition<decltype(params)>{query + " < ?", std::move(params)};
        }
        template <size_t N>
        auto operator>=(const char (&val)[N]) const {
            auto params = std::tuple_cat(this->params, std::tuple<std::string_view>{std::string_view(val, (-1))});
            return Condition<decltype(params)>{query + " >= ?", std::move(params)};
        }
        template <size_t N>
        auto operator<=(const char (&val)[N]) const {
            auto params = std::tuple_cat(this->params, std::tuple<std::string_view>{std::string_view(val, (-1))});
            return Condition<decltype(params)>{query + " <= ?", std::move(params)};
        }


/*
 * with primitive types
 */
#define ARITHMETIC_OPERATOR_FOR(T)                                                                                               \
    auto operator+(T val) const {                                                                                                \
        auto params = std::tuple_cat(this->params, std::tuple<T>{std::move(val)});                                               \
        return Arithmetics<decltype(params)>{"(" + query + " + ?)", std::move(params)};                                          \
    }                                                                                                                            \
    auto operator-(T val) const {                                                                                                \
        auto params = std::tuple_cat(this->params, std::tuple<T>{std::move(val)});                                               \
        return Arithmetics<decltype(params)>{"(" + query + " - ?)", std::move(params)};                                          \
    }                                                                                                                            \
    auto operator*(T val) const {                                                                                                \
        auto params = std::tuple_cat(this->params, std::tuple<T>{std::move(val)});                                               \
        return Arithmetics<decltype(params)>{"(" + query + " * ?)", std::move(params)};                                          \
    }                                                                                                                            \
    auto operator/(T val) const {                                                                                                \
        auto params = std::tuple_cat(this->params, std::tuple<T>{std::move(val)});                                               \
        return Arithmetics<decltype(params)>{"(" + query + " / ?)", std::move(params)};                                          \
    }                                                                                                                            \
    auto operator==(T val) const {                                                                                               \
        auto params = std::tuple_cat(this->params, std::tuple<T>{std::move(val)});                                               \
        return Condition<decltype(params)>{query + " = ?", std::move(params)};                                                   \
    }                                                                                                                            \
    auto operator!=(T val) const {                                                                                               \
        auto params = std::tuple_cat(this->params, std::tuple<T>{std::move(val)});                                               \
        return Condition<decltype(params)>{query + " != ?", std::move(params)};                                                  \
    }                                                                                                                            \
    auto operator>(T val) const {                                                                                                \
        auto params = std::tuple_cat(this->params, std::tuple<T>{std::move(val)});                                               \
        return Condition<decltype(params)>{query + " > ?", std::move(params)};                                                   \
    }                                                                                                                            \
    auto operator<(T val) const {                                                                                                \
        auto params = std::tuple_cat(this->params, std::tuple<T>{std::move(val)});                                               \
        return Condition<decltype(params)>{query + " < ?", std::move(params)};                                                   \
    }                                                                                                                            \
    auto operator>=(T val) const {                                                                                               \
        auto params = std::tuple_cat(this->params, std::tuple<T>{std::move(val)});                                               \
        return Condition<decltype(params)>{query + " >= ?", std::move(params)};                                                  \
    }                                                                                                                            \
    auto operator<=(T val) const {                                                                                               \
        auto params = std::tuple_cat(this->params, std::tuple<T>{std::move(val)});                                               \
        return Condition<decltype(params)>{query + " <= ?", std::move(params)};                                                  \
    }

        ARITHMETIC_OPERATOR_FOR(int)
        ARITHMETIC_OPERATOR_FOR(double)
        ARITHMETIC_OPERATOR_FOR(std::string)
        ARITHMETIC_OPERATOR_FOR(std::string_view)

#undef ARITHMETIC_OPERATOR_FOR
    };

    template <typename T>
    class Alias : public Column<cpx::sql::detail::NoTable, T> {
    protected:
        std::string qname;

    public:
        using type = T;

        Alias(std::string qualified_name)
            : qname(std::move(qualified_name)) {
            this->name_ = this->qname;
        }

        template <size_t N>
        Alias(const char (&qualified_name)[N])
            : Alias(std::string(qualified_name, N - 1)) {}
    };

    template <typename Table, typename T>
    class Column {
    protected:
        std::string_view name_;
        std::string_view column_;

        Column() = default;

    public:
        using type = T;

        constexpr Column(std::string_view column)
            : column_(column) {
            auto pos = column.find(' ');
            name_    = pos == std::string::npos ? column : column.substr(0, pos);
        }

        template <size_t N>
        constexpr Column(const char (&column)[N])
            : Column(std::string_view(column, N - 1)) {}

        void operator=(const char *) = delete;

        std::string column() const {
            return std::string(column_);
        }

        std::string name() const {
            return std::string(name_);
        }

        std::string qualified_name() const {
            const char    *table    = Table::TableName;
            constexpr bool is_tuple = cpx::is_tuple_v<T>;
            return (is_tuple ? "(" : "") +                                                              //
                   (table ? std::string(Table::TableName) + "." : std::string()) + std::string(name_) + //
                   (is_tuple ? ")" : "");
        }

        Alias<T> as(const Alias<T> &alias) const {
            return qualified_name() + " as " + alias.name();
        }

        template <typename Tbl, typename U>
        auto operator,(const Column<Tbl, U> &other) const {
            using R = decltype(std::tuple_cat(
                std::declval<std::conditional_t<cpx::is_tuple_v<T>, T, std::tuple<T>>>(),
                std::declval<std::conditional_t<cpx::is_tuple_v<U>, U, std::tuple<U>>>()
            ));
            return Alias<R>{qualified_name() + ", " + other.qualified_name()};
        }

        /*
         * with Self
         */

        template <typename Tbl, typename U>
        Assignment<> operator=(const Column<Tbl, U> &other) const {
            return {name() + " = " + other.qualified_name()};
        }
        template <typename Tbl, typename U>
        auto operator+(const Column<Tbl, U> &other) const {
            using R = decltype(std::declval<T>() + std::declval<U>());
            return Alias<R>('(' + qualified_name() + " + " + other.qualified_name() + ')');
        }
        template <typename Tbl, typename U>
        auto operator-(const Column<Tbl, U> &other) const {
            using R = decltype(std::declval<T>() - std::declval<U>());
            return Alias<R>('(' + qualified_name() + " - " + other.qualified_name() + ')');
        }
        template <typename Tbl, typename U>
        auto operator*(const Column<Tbl, U> &other) const {
            using R = decltype(std::declval<T>() - std::declval<U>());
            return Alias<R>('(' + qualified_name() + " * " + other.qualified_name() + ')');
        }
        template <typename Tbl, typename U>
        auto operator/(const Column<Tbl, U> &other) const {
            using R = decltype(std::declval<T>() - std::declval<U>());
            return Alias<R>('(' + qualified_name() + " / " + other.qualified_name() + ')');
        }
        template <typename Tbl, typename U>
        Condition<> operator==(const Column<Tbl, U> &other) const {
            return {qualified_name() + " = " + other.qualified_name()};
        }
        template <typename Tbl, typename U>
        Condition<> operator!=(const Column<Tbl, U> &other) const {
            return {qualified_name() + " != " + other.qualified_name()};
        }
        template <typename Tbl, typename U>
        Condition<> operator>(const Column<Tbl, U> &other) const {
            return {qualified_name() + " > " + other.qualified_name()};
        }
        template <typename Tbl, typename U>
        Condition<> operator<(const Column<Tbl, U> &other) const {
            return {qualified_name() + " < " + other.qualified_name()};
        }
        template <typename Tbl, typename U>
        Condition<> operator>=(const Column<Tbl, U> &other) const {
            return {qualified_name() + " >= " + other.qualified_name()};
        }
        template <typename Tbl, typename U>
        Condition<> operator<=(const Column<Tbl, U> &other) const {
            return {qualified_name() + " <= " + other.qualified_name()};
        }


        /*
         * with T
         */

        Assignment<std::tuple<T>> operator=(T val) const {
            return {name() + " = ?", {std::move(val)}};
        }
        Arithmetics<std::tuple<T>> operator+(T val) const {
            return {"(" + qualified_name() + " + ?)", {std::move(val)}};
        }
        Arithmetics<std::tuple<T>> operator-(T val) const {
            return {"(" + qualified_name() + " - ?)", {std::move(val)}};
        }
        Arithmetics<std::tuple<T>> operator*(T val) const {
            return {"(" + qualified_name() + " * ?)", {std::move(val)}};
        }
        Arithmetics<std::tuple<T>> operator/(T val) const {
            return {"(" + qualified_name() + " / ?)", {std::move(val)}};
        }
        Condition<std::tuple<T>> operator==(T val) const {
            return {qualified_name() + " = ?", {std::move(val)}};
        }
        Condition<std::tuple<T>> operator!=(T val) const {
            return {qualified_name() + " != ?", {std::move(val)}};
        }
        Condition<std::tuple<T>> operator>(T val) const {
            return {qualified_name() + " > ?", {std::move(val)}};
        }
        Condition<std::tuple<T>> operator<(T val) const {
            return {qualified_name() + " < ?", {std::move(val)}};
        }
        Condition<std::tuple<T>> operator>=(T val) const {
            return {qualified_name() + " >= ?", {std::move(val)}};
        }
        Condition<std::tuple<T>> operator<=(T val) const {
            return {qualified_name() + " <= ?", {std::move(val)}};
        }


        /*
         * with string literals
         */

        template <size_t N>
        Assignment<std::tuple<std::string_view>> operator=(const char (&val)[N]) const {
            static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>);
            return {name() + " = ?", {std::string_view(val, N - 1)}};
        }
        template <size_t N>
        Condition<std::tuple<std::string_view>> operator==(const char (&val)[N]) const {
            static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>);
            return {qualified_name() + " = ?", {std::string_view(val, N - 1)}};
        }
        template <size_t N>
        Condition<std::tuple<std::string_view>> operator!=(const char (&val)[N]) const {
            static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>);
            return {qualified_name() + " != ?", {std::string_view(val, N - 1)}};
        }
        template <size_t N>
        Condition<std::tuple<std::string_view>> operator>(const char (&val)[N]) const {
            static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>);
            return {qualified_name() + " > ?", {std::string_view(val, N - 1)}};
        }
        template <size_t N>
        Condition<std::tuple<std::string_view>> operator<(const char (&val)[N]) const {
            static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>);
            return {qualified_name() + " < ?", {std::string_view(val, N - 1)}};
        }
        template <size_t N>
        Condition<std::tuple<std::string_view>> operator>=(const char (&val)[N]) const {
            static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>);
            return {qualified_name() + " >= ?", {std::string_view(val, N - 1)}};
        }
        template <size_t N>
        Condition<std::tuple<std::string_view>> operator<=(const char (&val)[N]) const {
            static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>);
            return {qualified_name() + " <= ?", {std::string_view(val, N - 1)}};
        }


        /*
         * with Arithmetics
         */

        template <typename Params>
        Assignment<Params> operator=(Arithmetics<Params> ari) const {
            return {name() + " = " + ari.query, std::move(ari.params)};
        }
        template <typename Params>
        Arithmetics<Params> operator+(Arithmetics<Params> ari) const {
            return {"(" + qualified_name() + " + " + ari.query + ")", std::move(ari.params)};
        }
        template <typename Params>
        Arithmetics<Params> operator-(Arithmetics<Params> ari) const {
            return {"(" + qualified_name() + " - " + ari.query + ")", std::move(ari.params)};
        }
        template <typename Params>
        Arithmetics<Params> operator*(Arithmetics<Params> ari) const {
            return {"(" + qualified_name() + " * " + ari.query + ")", std::move(ari.params)};
        }
        template <typename Params>
        Arithmetics<Params> operator/(Arithmetics<Params> ari) const {
            return {"(" + qualified_name() + " / " + ari.query + ")", std::move(ari.params)};
        }
        template <typename Params>
        Condition<Params> operator==(Arithmetics<Params> ari) const {
            return {qualified_name() + " = " + ari.query, std::move(ari.params)};
        }
        template <typename Params>
        Condition<Params> operator!=(Arithmetics<Params> ari) const {
            return {qualified_name() + " != " + ari.query, std::move(ari.params)};
        }
        template <typename Params>
        Condition<Params> operator>(Arithmetics<Params> ari) const {
            return {qualified_name() + " > " + ari.query, std::move(ari.params)};
        }
        template <typename Params>
        Condition<Params> operator<(Arithmetics<Params> ari) const {
            return {qualified_name() + " < " + ari.query, std::move(ari.params)};
        }
        template <typename Params>
        Condition<Params> operator>=(Arithmetics<Params> ari) const {
            return {qualified_name() + " >= " + ari.query, std::move(ari.params)};
        }
        template <typename Params>
        Condition<Params> operator<=(Arithmetics<Params> ari) const {
            return {qualified_name() + " <= " + ari.query, std::move(ari.params)};
        }


        Alias<T> asc() const {
            return qualified_name() + " asc";
        }
        Alias<T> desc() const {
            return qualified_name() + " desc";
        }

        Condition<std::tuple<std::vector<T>>> in(std::vector<T> values) const {
            return {qualified_name() + " in " + get_placeholders(values.size()), {std::move(values)}};
        }
        Condition<std::tuple<std::vector<T>>> not_in(std::vector<T> values) const {
            return {qualified_name() + " not in " + get_placeholders(values.size()), {std::move(values)}};
        }
        template <typename Params, typename Row>
        Condition<Params> in(Statement<Params, Row> stmt) const {
            return {qualified_name() + " in (" + stmt.query + ")", std::move(stmt.params)};
        }
        template <typename Params, typename Row>
        Condition<Params> not_in(Statement<Params, Row> stmt) const {
            return {qualified_name() + " not in (" + stmt.query + ")", std::move(stmt.params)};
        }

        template <size_t N>
        Condition<std::tuple<std::string_view>> like(const char (&str)[N]) const {
            return {qualified_name() + " limit ?", {{str, N - 1}}};
        }
        Condition<std::tuple<std::string_view>> like(std::string_view str) const {
            return {qualified_name() + " limit ?", {str}};
        }
        Condition<std::tuple<std::string>> like(std::string &&str) const {
            return {qualified_name() + " limit ?", {std::move(str)}};
        }
        Condition<std::tuple<std::string_view>> like(const std::string &str) const {
            return {qualified_name() + " limit ?", {str}};
        }

    protected:
        static std::string get_placeholders(size_t size) {
            if (size == 0)
                return "()";

            std::string placeholder = "?";
            if constexpr (cpx::is_tuple_v<T>)
                placeholder = "(" + cpx::sql::detail::repeated_placeholders<std::tuple_size_v<T>>::value() + ")";

            std::string pch = "";
            for (size_t i = 0; i < size; i++)
                pch += (i == 0 ? "(" : ", ") + placeholder;
            pch += ')';

            return pch;
        }
    };

    // TODO: tuple reference?
    template <typename Params, typename Row>
    struct Statement {
        static_assert(cpx::is_tuple_v<Params> && cpx::is_tuple_v<Row>);

        using params_type = Params;
        using row_type    = Row;

        std::string query;
        Params      params = {};

        template <typename OParams, typename ORow>
        auto operator+(const Statement<OParams, ORow> &other) const {
            using P = decltype(std::tuple_cat(std::declval<Params>(), std::declval<OParams>()));
            using R = decltype(std::tuple_cat(std::declval<Row>(), std::declval<ORow>()));
            return Statement<P, R>{query + other.query, std::tuple_cat(params, other.params)};
        }

        template <typename... Tables, typename... Ts>
        auto select(const Column<Tables, Ts> &...cols) const {
            static_assert(sizeof...(cols) == std::tuple_size_v<Params>, "Number of columns must match the number of params");
            return Statement<>{query + " select " + cpx::sql::detail::string_cat(", ", cols.qualified_name()...)};
        };

        template <typename... OParams>
        auto set(const Assignment<OParams> &...assignments) const {
            return *this + Statement<>{" set "} + (assignments, ...).into_statement();
        }

        template <typename Table>
        auto from(const Table &table) const {
            return *this + Statement<>{std::string(" from ") + table.TableName};
        }

        template <typename Table>
        auto join(const Table &table) const {
            return *this + Statement<>{std::string(" join ") + table.TableName};
        }

        template <typename Table>
        auto left_join(const Table &table) const {
            return *this + Statement<>{std::string(" left join ") + table.TableName};
        }

        template <typename Table>
        auto inner_join(const Table &table) const {
            return *this + Statement<>{std::string(" inner join ") + table.TableName};
        }

        template <typename Table>
        auto right_join(const Table &table) const {
            return *this + Statement<>{std::string(" right join ") + table.TableName};
        }

        // for alter_table
        template <typename Table, typename T>
        auto add_column(const Column<Table, T> &col) const {
            return *this + Statement<>{std::string(" add ") + col.column()};
        }

        template <typename Table, typename T>
        auto drop_column(const Column<Table, T> &col) const {
            return *this + Statement<>{std::string(" drop column ") + col.name()};
        }

        template <typename OParams>
        auto where(const Condition<OParams> &condition) const {
            return *this + Statement<>{" where "} + condition.into_statement();
        }

        template <typename OParams>
        auto on(const Condition<OParams> &condition) const {
            return *this + Statement<>{" on "} + condition.into_statement();
        }

        auto values(Params params) const {
            auto pch  = detail::repeated_placeholders<std::tuple_size_v<Params>>::value();
            auto stmt = Statement<Params>{query + " values (" + pch + ")", std::move(params)};
            return stmt;
        }

        auto values(Params params, Params params2) const {
            auto pch   = detail::repeated_placeholders<std::tuple_size_v<Params>>::value();
            auto stmt  = Statement<Params>{query + " values (" + pch + ")", std::move(params)};
            auto stmt2 = Statement<Params>{", (" + pch + ")", std::move(params2)};
            return stmt + stmt2;
        }

        auto values(Params params, Params params2, Params params3) const {
            auto pch   = detail::repeated_placeholders<std::tuple_size_v<Params>>::value();
            auto stmt  = Statement<Params>{query + " values (" + pch + ")", std::move(params)};
            auto stmt2 = Statement<Params>{", (" + pch + ")", std::move(params2)};
            auto stmt3 = Statement<Params>{", (" + pch + ")", std::move(params3)};
            return stmt + stmt2 + stmt3;
        }

        auto values(std::vector<Params> params) const {
            const auto pch = '(' + detail::repeated_placeholders<std::tuple_size_v<Params>>::value() + ')';

            std::string pchs = "";
            for (size_t i = 0; i < params.size(); i++)
                pchs += (i == 0 ? "" : ", ") + pch;

            auto stmt = Statement<std::tuple<std::vector<Params>>>{query + " values " + pchs, {std::move(params)}};
            return stmt;
        }

        template <typename... Tables, typename... Ts>
        auto order_by(const Column<Tables, Ts> &...cols) const {
            return *this + Statement<>{" order by " + cpx::sql::detail::string_cat(", ", cols.qualified_name()...)};
        };

        auto limit(std::optional<size_t> val) const {
            if (val.has_value())
                return *this + Statement<>{" limit " + std::to_string(*val)};
            else
                return *this;
        };

        auto offset(std::optional<size_t> val) const {
            if (val.has_value())
                return *this + Statement<>{" offset " + std::to_string(*val)};
            else
                return *this;
        };
    };

    template <const auto &table, typename... Tables, typename... Ts>
    auto create_table(const Column<Tables, Ts> &...cols) {
        auto q = std::string("create table ") + table.TableName + " ";
        q += "(" + cpx::sql::detail::string_cat(", ", cols.column()...) + ")";
        return Statement<>{std::move(q)};
    }

    template <const auto &table, typename... Tables, typename... Ts>
    auto create_table_if_not_exists(const Column<Tables, Ts> &...cols) {
        auto q = std::string("create table if not exists ") + table.TableName + " ";
        q += "(" + cpx::sql::detail::string_cat(", ", cols.column()...) + ")";
        return Statement<>{std::move(q)};
    }

    template <const auto &table>
    inline const Statement<> alter_table = {std::string("alter table ") + table.TableName};

    template <const auto &table>
    inline const Statement<> update = {std::string("update ") + table.TableName};

    template <const auto &table, typename... Tables, typename... Ts>
    auto insert_into(const Column<Tables, Ts> &...cols) {
        auto q = std::string("insert into ") + table.TableName + " (" + cpx::sql::detail::string_cat(", ", cols.name()...) + ")";
        return Statement<std::tuple<Ts...>>{std::move(q)};
    }

    // TODO: select literals?
    template <typename... Tables, typename... Ts>
    auto select(const Column<Tables, Ts> &...cols) {
        auto q = std::string("select ") + cpx::sql::detail::string_cat(", ", cols.qualified_name()...);
        return Statement<std::tuple<>, std::tuple<Ts...>>{std::move(q)};
    };

#ifdef BOOST_PFR_HPP
    template <typename Table>
    auto select_all_from(const Table &table) {
        using TupleStruct = decltype(boost::pfr::structure_to_tuple(table));
        using Filtered    = cpx::filter_tuple_t<TupleStruct, cpx::sql::detail::is_column>;
        using Tuple       = cpx::apply_tuple_t<Filtered, cpx::sql::detail::column>;
        return Statement<std::tuple<>, Tuple>{std::string("select * from ") + table.TableName};
    };
#endif

    template <typename Table>
    auto delete_from(const Table &table) {
        return Statement<>{std::string("delete from ") + table.TableName};
    };
} // namespace cpx::sql
#endif
