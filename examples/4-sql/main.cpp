#include <cpx/fmt.h>
#include <cpx/sql/sql.h>

namespace sql = cpx::sql;

struct User {
    static constexpr const char *TableName = "users";

    sql::Column<User, int>         id   = "id integer primary key";
    sql::Column<User, std::string> name = "name varchar(32) not null";
    sql::Column<User, int>         age  = "age integer";
};
static constexpr User users;

struct Product {
    static constexpr const char *TableName = "products";

    sql::Column<Product, int>    id    = "id integer primary key";
    sql::Column<Product, double> price = "price real";
    sql::Column<Product, int>    stock = "stock integer";
};
static constexpr Product products;

int main() {
    {
        auto stmt = sql::create_table<products>( //
            products.id,
            products.price,
            products.stock
        );
        fmt::println("{}", stmt);
    }
    {
        auto stmt = sql::create_table<users>( //
            users.id,
            users.name,
            users.age
        );
        fmt::println("{}", stmt);
    }

    {
        auto stmt = sql::update<products> //
                        .set( //
                            products.price = 9.99, 
                            products.stock = products.stock - 10
                        )
                        .where(products.id == 42);
        fmt::println("{}", stmt);
    }
    {
        auto stmt = sql::insert_into<users>(users.name, users.age) //
                        .values({"Sucipto", 20}, {"Sugeng", 25});
        fmt::println("{}", stmt);
    }
    {
        auto stmt = sql::select(users.id, users.name, users.age) //
                        .from(users)
                        .where(users.name == "Sugeng");
        fmt::println("{}", stmt);
    }
    {
        auto stmt = sql::select(products.price, products.stock)
                        .from(products)
                        .where(products.price > 4.99 || products.stock <= 10)
                        .order_by(products.stock, products.price.desc());
        fmt::println("{}", stmt);
    }
    {
        auto stmt = sql::insert_into<users>(users.id, users.age)
                        .select(products.id, products.stock)
                        .from(products)
                        .where(products.id == 42);
        fmt::println("{}", stmt);
    }
    {
        auto asdf = products.id;
        auto stmt = sql::select(users.name, products.stock) //
                        .from(users)
                        .left_join(products)
                        .on(users.id == asdf);
        fmt::println("{}", stmt);
    }
}
