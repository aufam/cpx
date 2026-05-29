#ifndef CPX_SQL_SQL_H
#define CPX_SQL_SQL_H

#include <cpx/tuple.h>
#include <optional>
#include <string>
#include <utility>
#include <vector>

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

    template <typename T>
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

    // column type
    template <typename T>
    struct column;

    template <typename T>
    struct column<sql::Column<T>> {
        using type = typename sql::Column<T>::type;
    };

    template <typename T>
    using column_t = typename column<T>::type;

    // is_column
    template <typename T>
    struct is_column : std::false_type {};

    template <typename T>
    struct is_column<Column<T>> : std::true_type {};

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
    std::string string_cat(const std::string &) {
        return "";
    }

    template <typename Arg, typename... Args>
    std::string string_cat(const std::string &separator, Arg &&arg, Args &&...args) {
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
        using params_type = Params;
        using row_type    = Row;

        std::string query;
        Params      params = {};

        template <typename Other>
        auto operator+(const Other &other) const {
            return Statement<
                decltype(std::tuple_cat(params_type{}, typename Other::params_type{})),
                decltype(std::tuple_cat(row_type{}, typename Other::row_type{}))>{
                query + other.query, std::tuple_cat(params, other.params)
            };
        }

        template <typename Other>
        auto operator&&(const Other &other) const {
            return Statement<>{"("} + *this + Statement<>{" and "} + other + Statement<>{")"};
        }

        template <typename Other>
        auto operator||(const Other &other) const {
            return Statement<>{"("} + *this + Statement<>{" or "} + other + Statement<>{")"};
        }

        auto operator!() const {
            return Statement<>{"not ("} + *this + Statement<>{")"};
        }

        template <typename... Cols>
        auto select(const Cols &...cols) const {
            static_assert(sizeof...(cols) == std::tuple_size_v<Params>, "Number of columns must match the number of params");
            return Statement<>{query + " select " + cpx::sql::detail::string_cat(", ", cols.name()...)};
        };

        template <typename Other, typename... Rest>
        auto set(const Other &other, const Rest &...rest) const {
            if constexpr (sizeof...(Rest) == 0)
                return *this + Statement<>{" set "} + other;
            else
                return *this + Statement<>{" set "} + other + ((Statement<>{", "} + rest) + ...);
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

        template <typename Col>
        auto to(const Col &col) const {
            return *this + Statement<>{std::string(" to ") + col.name()};
        }

        template <typename Col>
        auto add(const Col &col) const {
            return *this + Statement<>{std::string(" add ") + col.name()};
        }

        template <typename Col>
        auto drop_column(const Col &col) const {
            return *this + Statement<>{std::string(" drop column ") + col.name()};
        }

        template <typename Condition>
        auto where(const Condition &condition) const {
            return *this + Statement<>{" where "} + condition;
        }

        template <typename Condition>
        auto on(const Condition &condition) const {
            return *this + Statement<>{" on "} + condition;
        }

        template <typename... Rest>
        auto values(const Params &params, const Rest &...res) const {
            if constexpr (sizeof...(res) == 0)
                return Statement<Params>{
                    query + " values (" + detail::repeated_placeholders<std::tuple_size_v<Params>>::value() + ")", params
                };
            else
                return Statement<Params>{
                           query + " values (" + detail::repeated_placeholders<std::tuple_size_v<Params>>::value() + ")", params
                       } +
                       ((Statement<Params>{", (" + detail::repeated_placeholders<std::tuple_size_v<Params>>::value() + ")", res}
                        ) +
                        ...);
        }

        template <typename Col, typename... Cols>
        auto order_by(const Col &col, const Cols &...cols) const {
            if constexpr (sizeof...(Cols) == 0)
                return *this + Statement<>{" order by " + col.name()};
            else
                return *this + Statement<>{" order by " + col.name() + ((std::string(", ") + cols.name()) + ...)};
        };

        auto order_by(const std::vector<std::string> &cols) const {
            if (!cols.empty()) {
                std::string clause = " order by " + cols[0];
                for (size_t i = 1; i < cols.size(); ++i) {
                    clause += ", " + cols[i];
                }
                return *this + Statement<>{clause};
            } else {
                return *this;
            }
        }

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

    template <typename T = void>
    class Alias {
    protected:
        std::string name_;

    public:
        using type = T;

        explicit Alias(std::string name)
            : name_(std::move(name)) {}

        const std::string name() const {
            return name_;
        }

        template <typename U>
        auto as(const Alias<U> &alias) {
            return Alias<T>(name() + " as " + alias.name());
        }
    };

    Alias(std::string) -> Alias<void>;

    template <typename T>
    class Column {
    protected:
        std::string_view name_;
        std::string_view column_;

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

        template <typename U>
        auto as(const Alias<T> &alias) {
            return name() + " as " + alias.name();
        }

        template <typename U>
        auto operator+(const Column<U> &other) const {
            return Alias<decltype(std::declval<T>() + std::declval<U>())>('(' + name() + " + " + other.name() + ')');
        }

        template <typename U>
        auto operator-(const Column<U> &other) const {
            return Alias<decltype(std::declval<T>() - std::declval<U>())>(name() + " - " + other.name());
        }

        template <typename U>
        auto operator*(const Column<U> &other) const {
            return Alias<decltype(std::declval<T>() * std::declval<U>())>(name() + " * " + other.name());
        }

        template <typename U>
        auto operator/(const Column<U> &other) const {
            return Alias<decltype(std::declval<T>() / std::declval<U>())>(name() + " / " + other.name());
        }

        Statement<std::tuple<T>> operator=(const T &val) const {
            return {name() + " = ?", {val}};
        }
        Statement<std::tuple<T>> operator+(const T &val) const {
            return {name() + " + ?", {val}};
        }
        Statement<std::tuple<T>> operator-(const T &val) const {
            return {name() + " - ?", {val}};
        }
        Statement<std::tuple<T>> operator*(const T &val) const {
            return {name() + " * ?", {val}};
        }
        Statement<std::tuple<T>> operator/(const T &val) const {
            return {name() + " / ?", {val}};
        }

        Statement<std::tuple<T>> operator==(const T &val) const {
            return {name() + " = ?", {val}};
        }
        Statement<std::tuple<T>> operator!=(const T &val) const {
            return {name() + " != ?", {val}};
        }
        Statement<std::tuple<T>> operator>(const T &val) const {
            return {name() + " > ?", {val}};
        }
        Statement<std::tuple<T>> operator<(const T &val) const {
            return {name() + " < ?", {val}};
        }
        Statement<std::tuple<T>> operator>=(const T &val) const {
            return {name() + " >= ?", {val}};
        }
        Statement<std::tuple<T>> operator<=(const T &val) const {
            return {name() + " <= ?", {val}};
        }

        template <typename Params, typename Row>
        auto operator=(const Statement<Params, Row> &stmt) const {
            return Statement<>{name() + " = "} + stmt;
        }

        template <typename Params, typename Row>
        auto operator+(const Statement<Params, Row> &stmt) const {
            return Statement<>{name() + " + "} + stmt;
        }

        template <typename Params, typename Row>
        auto operator-(const Statement<Params, Row> &stmt) const {
            return Statement<>{name() + " - "} + stmt;
        }

        template <typename Params, typename Row>
        auto operator*(const Statement<Params, Row> &stmt) const {
            return Statement<>{name() + " * "} + stmt;
        }

        template <typename Params, typename Row>
        auto operator/(const Statement<Params, Row> &stmt) const {
            return Statement<>{name() + " / "} + stmt;
        }

        template <typename Params, typename Row>
        auto operator==(const Statement<Params, Row> &stmt) const {
            return Statement<>{name() + " = "} + stmt;
        }

        template <typename Params, typename Row>
        auto operator!=(const Statement<Params, Row> &stmt) const {
            return Statement<>{name() + " != "} + stmt;
        }

        template <typename Params, typename Row>
        auto operator>(const Statement<Params, Row> &stmt) const {
            return Statement<>{name() + " > "} + stmt;
        }

        template <typename Params, typename Row>
        auto operator<(const Statement<Params, Row> &stmt) const {
            return Statement<>{name() + " < "} + stmt;
        }

        template <typename Params, typename Row>
        auto operator>=(const Statement<Params, Row> &stmt) const {
            return Statement<>{name() + " >= "} + stmt;
        }

        template <typename Params, typename Row>
        auto operator<=(const Statement<Params, Row> &stmt) const {
            return Statement<>{name() + " <= "} + stmt;
        }

        /// TODO: just to avoid ambiguity with `table.col == "literal"`
        auto operator==(Column<T> &other) const {
            return Statement<>{name() + " = " + other.name()};
        }

        struct Asc {
            const Column *col;
            std::string   name() const {
                return col->name() + " asc";
            }
        };
        Asc asc() const {
            return {this};
        }

        struct Desc {
            const Column *col;
            std::string   name() const {
                return col->name() + " desc";
            }
        };
        Desc desc() const {
            return {this};
        }
    };

    template <const auto &table, typename... Cols>
    auto create_table(const Cols &...cols) {
        return Statement<>{
            std::string("create table ") + table.TableName +     //
            " (" +                                               //
            cpx::sql::detail::string_cat(", ", cols.column()...) //
            + ")"
        };
    }

    template <const auto &table, typename... Cols>
    auto create_table_if_not_exists(const Cols &...cols) {
        return Statement<>{
            std::string("create table if not exists ") + table.TableName + //
            " (" +                                                         //
            cpx::sql::detail::string_cat(", ", cols.column()...) +         //
            ")"
        };
    }

    template <const auto &table>
    inline const Statement<> alter_table = {std::string("alter table ") + table.TableName};

    template <const auto &table>
    inline const Statement<> update = {std::string("update ") + table.TableName};

    template <const auto &table, typename... Cols>
    auto insert_into(const Cols &...cols) {
        return Statement<std::tuple<typename Cols::type...>>{
            std::string("insert into ") + table.TableName + " (" + cpx::sql::detail::string_cat(", ", cols.name()...) + ")"
        };
    }

    template <typename... Cols>
    auto select(const Cols &...cols) {
        return Statement<std::tuple<>, std::tuple<typename Cols::type...>>{
            std::string("select ") + cpx::sql::detail::string_cat(", ", cols.name()...)
        };
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
