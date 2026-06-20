module;

#include <cpx/sql/sql.h>

export module cpx.sql;
import cpx;

export namespace cpx::sql {
    using ::cpx::sql::Alias;
    using ::cpx::sql::Column;
    using ::cpx::sql::Connection;
    using ::cpx::sql::Rows;
    using ::cpx::sql::Statement;

    using ::cpx::sql::alter_table;
    using ::cpx::sql::create_table;
    using ::cpx::sql::create_table_if_not_exists;
    using ::cpx::sql::delete_from;
    using ::cpx::sql::insert_into;
    using ::cpx::sql::select;
    using ::cpx::sql::update;

#ifdef BOOST_PFR_HPP
    using ::cpx::sql::select_all_from;
#endif
} // namespace cpx::sql
