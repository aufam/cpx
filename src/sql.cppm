module;

#include <cpx/sql/sql.h>

export module cpx.sql;
import cpx;

export namespace cpx {
    template <typename T>
    struct is_tagged<sql::Column<T>>;

    template <typename T>
    struct remove_tag<sql::Column<T>>;
} // namespace cpx

export namespace cpx::sql {
    using ::cpx::sql::Connection;

    using ::cpx::sql::Schema;

    using ::cpx::sql::Constant;

    using ::cpx::sql::Rows;

    using ::cpx::sql::Statement;

    using ::cpx::sql::Column;

    using ::cpx::sql::Alias;

    using ::cpx::sql::create_table;
    using ::cpx::sql::create_table_if_not_exists;

    using ::cpx::sql::alter_table;

    using ::cpx::sql::update;

    using ::cpx::sql::insert_into;

    using ::cpx::sql::select;
    using ::cpx::sql::select_all_from;

    using ::cpx::sql::delete_from;
} // namespace cpx::sql
