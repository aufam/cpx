#ifndef CPX_SQL_SQL_H
#define CPX_SQL_SQL_H

#include <cpx/tuple.h>
#include <optional>
#include <string>
#include <utility>

#ifndef BOOST_PFR_HPP
#    if __has_include(<boost/pfr.hpp>)
#        include <boost/pfr.hpp>
#    endif
#endif

// TODO: implement perfect forwarding


/*
 * Forward declarations
 */
namespace cpx::sql {
    class Connection;

    template <typename Row>
    class Rows;

    template <typename Params, typename Row>
    struct Statement;

    template <typename Table, typename T>
    class Column;

    template <typename T>
    class Alias;
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

    template <typename T>
    using column_t = typename column<T>::type;

    // is_column
    template <typename T>
    struct is_column : std::false_type {};

    template <typename Table, typename T>
    struct is_column<Column<Table, T>> : std::true_type {};

    template <typename T>
    inline constexpr bool is_column_v = is_column<T>::value;

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

    // TODO: tuple reference?
    template <typename Params = std::tuple<>, typename Row = std::tuple<>>
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

        template <typename OParams, typename ORow>
        auto operator&&(const Statement<OParams, ORow> &other) const {
            return Statement<>{"("} + *this + Statement<>{" and "} + other + Statement<>{")"};
        }

        template <typename OParams, typename ORow>
        auto operator||(const Statement<OParams, ORow> &other) const {
            return Statement<>{"("} + *this + Statement<>{" or "} + other + Statement<>{")"};
        }

        auto operator!() const {
            return Statement<>{"not ("} + *this + Statement<>{")"};
        }

        template <typename... Tables, typename... Ts>
        auto select(const Column<Tables, Ts> &...cols) const {
            static_assert(sizeof...(cols) == std::tuple_size_v<Params>, "Number of columns must match the number of params");
            return Statement<>{query + " select " + cpx::sql::detail::string_cat(", ", cols.qualified_name()...)};
        };

        template <typename... OParams, typename... ORow>
        auto set(const Statement<OParams, ORow> &...others) const {
            return *this + Statement<>{" set "} + cpx::sql::detail::string_cat(Statement<>{", "}, others...);
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

        template <typename OParams, typename ORow>
        auto where(const Statement<OParams, ORow> &condition) const {
            return *this + Statement<>{" where "} + condition;
        }

        template <typename OParams, typename ORow>
        auto on(const Statement<OParams, ORow> &condition) const {
            return *this + Statement<>{" on "} + condition;
        }

        auto values(const Params &params) const {
            auto pch  = detail::repeated_placeholders<std::tuple_size_v<Params>>::value();
            auto stmt = Statement<Params>{query + " values (" + pch + ")", params};
            return stmt;
        }

        auto values(const Params &params, const Params &params2) const {
            auto pch   = detail::repeated_placeholders<std::tuple_size_v<Params>>::value();
            auto stmt  = Statement<Params>{query + " values (" + pch + ")", params};
            auto stmt2 = Statement<Params>{", (" + pch + ")", params2};
            return stmt + stmt2;
        }

        auto values(const Params &params, const Params &params2, const Params &params3) const {
            auto pch   = detail::repeated_placeholders<std::tuple_size_v<Params>>::value();
            auto stmt  = Statement<Params>{query + " values (" + pch + ")", params};
            auto stmt2 = Statement<Params>{", (" + pch + ")", params2};
            auto stmt3 = Statement<Params>{", (" + pch + ")", params3};
            return stmt + stmt2 + stmt3;
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
            const char *table = Table::TableName;
            return (table ? std::string(Table::TableName) + "." : std::string()) + std::string(name_);
        }

        Alias<T> as(const Alias<T> &alias) const {
            return qualified_name() + " as " + alias.name();
        }


        /*
         * with other column
         */

        template <typename Tbl, typename U>
        auto operator=(const Column<Tbl, U> &other) const {
            return Statement<>{name() + " = " + other.qualified_name()};
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
        auto operator==(const Column<Tbl, U> &other) const {
            return Statement<>{qualified_name() + " = " + other.qualified_name()};
        }
        template <typename Tbl, typename U>
        auto operator!=(const Column<Tbl, U> &other) const {
            return Statement<>{qualified_name() + " != " + other.qualified_name()};
        }
        template <typename Tbl, typename U>
        auto operator>(const Column<Tbl, U> &other) const {
            return Statement<>{qualified_name() + " > " + other.qualified_name()};
        }
        template <typename Tbl, typename U>
        auto operator<(const Column<Tbl, U> &other) const {
            return Statement<>{qualified_name() + " < " + other.qualified_name()};
        }
        template <typename Tbl, typename U>
        auto operator>=(const Column<Tbl, U> &other) const {
            return Statement<>{qualified_name() + " >= " + other.qualified_name()};
        }
        template <typename Tbl, typename U>
        auto operator<=(const Column<Tbl, U> &other) const {
            return Statement<>{qualified_name() + " <= " + other.qualified_name()};
        }


        /*
         * with T
         */

        Statement<std::tuple<T>> operator=(const T &val) const {
            return {name() + " = ?", {val}};
        }
        Statement<std::tuple<T>> operator+(const T &val) const {
            return {qualified_name() + " + ?", {val}};
        }
        Statement<std::tuple<T>> operator-(const T &val) const {
            return {qualified_name() + " - ?", {val}};
        }
        Statement<std::tuple<T>> operator*(const T &val) const {
            return {qualified_name() + " * ?", {val}};
        }
        Statement<std::tuple<T>> operator/(const T &val) const {
            return {qualified_name() + " / ?", {val}};
        }
        Statement<std::tuple<T>> operator==(const T &val) const {
            return {qualified_name() + " = ?", {val}};
        }
        Statement<std::tuple<T>> operator!=(const T &val) const {
            return {qualified_name() + " != ?", {val}};
        }
        Statement<std::tuple<T>> operator>(const T &val) const {
            return {qualified_name() + " > ?", {val}};
        }
        Statement<std::tuple<T>> operator<(const T &val) const {
            return {qualified_name() + " < ?", {val}};
        }
        Statement<std::tuple<T>> operator>=(const T &val) const {
            return {qualified_name() + " >= ?", {val}};
        }
        Statement<std::tuple<T>> operator<=(const T &val) const {
            return {qualified_name() + " <= ?", {val}};
        }


        /*
         * with string literals
         */

        template <size_t N>
        Statement<std::tuple<std::string>> operator=(const char (&val)[N]) const {
            static_assert(std::is_same_v<T, std::string>);
            return {name() + " = ?", {val}};
        }
        template <size_t N>
        Statement<std::tuple<std::string>> operator==(const char (&val)[N]) const {
            static_assert(std::is_same_v<T, std::string>);
            return {qualified_name() + " = ?", {val}};
        }
        template <size_t N>
        Statement<std::tuple<std::string>> operator!=(const char (&val)[N]) const {
            static_assert(std::is_same_v<T, std::string>);
            return {qualified_name() + " != ?", {val}};
        }
        template <size_t N>
        Statement<std::tuple<std::string>> operator>(const char (&val)[N]) const {
            static_assert(std::is_same_v<T, std::string>);
            return {qualified_name() + " > ?", {val}};
        }
        template <size_t N>
        Statement<std::tuple<std::string>> operator<(const char (&val)[N]) const {
            static_assert(std::is_same_v<T, std::string>);
            return {qualified_name() + " < ?", {val}};
        }
        template <size_t N>
        Statement<std::tuple<std::string>> operator>=(const char (&val)[N]) const {
            static_assert(std::is_same_v<T, std::string>);
            return {qualified_name() + " >= ?", {val}};
        }
        template <size_t N>
        Statement<std::tuple<std::string>> operator<=(const char (&val)[N]) const {
            static_assert(std::is_same_v<T, std::string>);
            return {qualified_name() + " <= ?", {val}};
        }


        /*
         * with statement
         */

        template <typename Params, typename Row>
        auto operator=(const Statement<Params, Row> &stmt) const {
            return Statement<>{name() + " = "} + stmt;
        }
        template <typename Params, typename Row>
        auto operator+(const Statement<Params, Row> &stmt) const {
            return Statement<>{qualified_name() + " + "} + stmt;
        }
        template <typename Params, typename Row>
        auto operator-(const Statement<Params, Row> &stmt) const {
            return Statement<>{qualified_name() + " - "} + stmt;
        }
        template <typename Params, typename Row>
        auto operator*(const Statement<Params, Row> &stmt) const {
            return Statement<>{qualified_name() + " * "} + stmt;
        }
        template <typename Params, typename Row>
        auto operator/(const Statement<Params, Row> &stmt) const {
            return Statement<>{qualified_name() + " / "} + stmt;
        }
        template <typename Params, typename Row>
        auto operator==(const Statement<Params, Row> &stmt) const {
            return Statement<>{qualified_name() + " = "} + stmt;
        }
        template <typename Params, typename Row>
        auto operator!=(const Statement<Params, Row> &stmt) const {
            return Statement<>{qualified_name() + " != "} + stmt;
        }
        template <typename Params, typename Row>
        auto operator>(const Statement<Params, Row> &stmt) const {
            return Statement<>{qualified_name() + " > "} + stmt;
        }
        template <typename Params, typename Row>
        auto operator<(const Statement<Params, Row> &stmt) const {
            return Statement<>{qualified_name() + " < "} + stmt;
        }
        template <typename Params, typename Row>
        auto operator>=(const Statement<Params, Row> &stmt) const {
            return Statement<>{qualified_name() + " >= "} + stmt;
        }
        template <typename Params, typename Row>
        auto operator<=(const Statement<Params, Row> &stmt) const {
            return Statement<>{qualified_name() + " <= "} + stmt;
        }


        Alias<T> asc() const {
            return qualified_name() + " asc";
        }
        Alias<T> desc() const {
            return qualified_name() + " desc";
        }
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
