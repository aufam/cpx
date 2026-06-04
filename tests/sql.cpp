#include <cpx/sql/sql.h>
#include <gtest/gtest.h>

namespace sql = cpx::sql;

namespace {
    struct User {
        [[maybe_unused]]
        static constexpr const char *TableName = "users";

        sql::Column<User, int>         id   = "id integer primary key";
        sql::Column<User, std::string> name = "name varchar(32) not null";
        sql::Column<User, int>         age  = "age integer";
    };
    static constexpr User users;

    struct Product {
        [[maybe_unused]]
        static constexpr const char *TableName = "products";

        sql::Column<Product, int>    id    = "id integer primary key";
        sql::Column<Product, double> price = "price real";
        sql::Column<Product, int>    stock = "stock integer";
    };
    static constexpr Product products;
} // namespace

TEST(sql, create_table) {
    auto u = sql::create_table<users>(users.id, users.name, users.age);
    EXPECT_EQ(u.query, "create table users (id integer primary key, name varchar(32) not null, age integer)");
    EXPECT_EQ(u.params, std::tuple<>{});
    EXPECT_EQ(decltype(u)::row_type{}, std::tuple<>{});

    auto p = sql::create_table<products>(products.id, products.price, products.stock);
    EXPECT_EQ(p.query, "create table products (id integer primary key, price real, stock integer)");
    EXPECT_EQ(p.params, std::tuple<>{});
    EXPECT_EQ(decltype(p)::row_type{}, std::tuple<>{});
}


TEST(sql, update) {
    auto p = sql::update<products>                                                 //
                 .set(products.price = 9.99, products.stock = products.stock - 10) //
                 .where(products.id == 42);

    EXPECT_EQ(p.query, "update products set price = ?, stock = (products.stock - ?) where products.id = ?");
    EXPECT_EQ(p.params, (std::tuple<double, int, int>{9.99, 10, 42}));
    EXPECT_EQ(decltype(p)::row_type{}, std::tuple<>{});
}

TEST(sql, insert) {
    auto u = //
        sql::insert_into<users>(users.name, users.age)
            .values({
                {"Sucipto", 20},
                {"Sugeng",  25}
    });
    EXPECT_EQ(u.query, "insert into users (name, age) values (?, ?), (?, ?)");
    EXPECT_EQ(
        u.params,
        (std::tuple<std::vector<std::tuple<std::string, int>>>{
            {{"Sucipto", 20}, {"Sugeng", 25}}
    })
    );
    EXPECT_EQ(decltype(u)::row_type{}, std::tuple<>{});
}


TEST(sql, select) {
    auto s = sql::select_all_from(users).where(users.name == "Sugeng");

    EXPECT_EQ(s.query, "select * from users where users.name = ?");
    EXPECT_EQ(s.params, (std::tuple<std::string>{"Sugeng"}));
    EXPECT_EQ(decltype(s)::row_type{}, (std::tuple<int, std::string, int>{}));

    auto p = sql::select(products.price, products.stock)
                 .from(products)
                 .where(products.price > 4.99 || products.stock <= 10)
                 .order_by(products.stock, products.price.desc());

    EXPECT_EQ(
        p.query,
        "select products.price, products.stock "
        "from products "
        "where (products.price > ? or products.stock <= ?) "
        "order by products.stock, products.price desc"
    );
    EXPECT_EQ(p.params, (std::tuple<double, int>{4.99, 10}));
    EXPECT_EQ(decltype(p)::row_type{}, (std::tuple<double, int>{}));
}

TEST(sql, insert_from_select) {
    auto s = sql::insert_into<users>(users.id, users.age) //
                 .select(products.id, products.stock)     //
                 .from(products)                          //
                 .where(products.id == 42);

    EXPECT_EQ(s.query, "insert into users (id, age) select products.id, products.stock from products where products.id = ?");
    EXPECT_EQ(s.params, (std::tuple<int>{42}));
    EXPECT_EQ(decltype(s)::row_type{}, std::tuple<>{});
}
