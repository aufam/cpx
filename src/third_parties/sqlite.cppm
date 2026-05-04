module;

#include <cpx/sql/sqlite3.h>

export module cpx.sqlite;
import cpx;

export namespace cpx::sql::sqlite3 {
    using ::cpx::sql::sqlite3::Connection;
    using ::cpx::sql::sqlite3::Rows;

    using ::cpx::sql::sqlite3::Deserialize;
    using ::cpx::sql::sqlite3::Serialize;

    using ::cpx::sql::sqlite3::error;
} // namespace cpx::sql::sqlite3

export namespace cpx {
    namespace sqlite = ::cpx::sql::sqlite3;
}
