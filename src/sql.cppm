module;

#include <cpx/sql/sql.h>

export module cpx.sql;
import cpx;

export namespace cpx::sql {
    using ::cpx::sql::Alias;
    using ::cpx::sql::Column;
    using ::cpx::sql::Connection;
    using ::cpx::sql::Constant;
    using ::cpx::sql::Rows;
    using ::cpx::sql::Schema;
    using ::cpx::sql::Statement;

#ifdef BOOST_PFR_HPP
    using ::cpx::sql::create_table;
    using ::cpx::sql::create_table_if_not_exists;
#endif

    using ::cpx::sql::alter_table;
    using ::cpx::sql::delete_from;
    using ::cpx::sql::insert_into;
    using ::cpx::sql::select;
    using ::cpx::sql::select_all_from;
    using ::cpx::sql::update;
} // namespace cpx::sql
