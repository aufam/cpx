#include <cpx/sql/postgres.h>
#include <cpx/fmt.h>

namespace sql      = cpx::sql;
namespace postgres = cpx::postgres;

struct User {
    static constexpr const char *TableName = "users";

    sql::Column<User, long long>        id         = "id bigserial primary key";
    sql::Column<User, std::string_view> name       = "name text not null";
    sql::Column<User, int64_t>          age        = "age bigint not null check (age >= 0)";
    sql::Column<User, std::tm>          created_at = "created_at timestamptz not null default now()";
};
static constexpr User users;

int main() {
    postgres::Connection db("host=localhost port=5432 dbname=db user=user password=password");

    db(sql::create_table_if_not_exists<users>(users.id, users.name, users.age, users.created_at));

    fmt::println("insert");
    {
        // auto tm   = cpx::tm_from_string("2026-06-08T15:56:00Z");
        auto stmt = sql::insert_into<users>(users.name, users.age).values({"Sucipto", 24});
        auto row  = db(stmt);
    }

    fmt::println("");
    fmt::println("select");
    {
        auto stmt = sql::select(users.name, users.created_at, users.age).from(users).where(users.name == "Sucipto");
        for (auto row = db(stmt); !row.is_done(); row.next()) {
            auto cols = row.get();
            fmt::println("{}", cols);
        }
    }
}
