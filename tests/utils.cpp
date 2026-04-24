#include <cpx/optional.h>
#include <cpx/result.h>
#include <cpx/overload.h>
#include <cpx/iter.h>
#include <gtest/gtest.h>

#include <string>
#include <variant>
#include <vector>

using namespace cpx;

// ─── optional monadic operations ──────────────────────────────────────────────

TEST(Optional, AndThen_HasValue) {
    std::optional<int> opt = 21;
    auto result = cpx::and_then(opt, [](int n) -> std::optional<int> { return n * 2; });
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST(Optional, AndThen_Empty) {
    std::optional<int> opt;
    auto result = cpx::and_then(opt, [](int n) -> std::optional<int> { return n * 2; });
    EXPECT_FALSE(result.has_value());
}

TEST(Optional, AndThen_ReturnsNullopt) {
    std::optional<int> opt = 5;
    auto result = cpx::and_then(opt, [](int) -> std::optional<int> { return std::nullopt; });
    EXPECT_FALSE(result.has_value());
}

TEST(Optional, Transform_HasValue) {
    std::optional<int> opt = 21;
    auto result = cpx::transform(opt, [](int n) { return n * 2; });
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST(Optional, Transform_Empty) {
    std::optional<int> opt;
    auto result = cpx::transform(opt, [](int n) { return n * 2; });
    EXPECT_FALSE(result.has_value());
}

TEST(Optional, Transform_TypeChange) {
    std::optional<int> opt = 42;
    auto result = cpx::transform(opt, [](int n) { return std::to_string(n); });
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "42");
}

TEST(Optional, OrElse_HasValue) {
    std::optional<int> opt = 7;
    auto result = cpx::or_else(opt, []() -> std::optional<int> { return 99; });
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 7);
}

TEST(Optional, OrElse_Empty) {
    std::optional<int> opt;
    auto result = cpx::or_else(opt, []() -> std::optional<int> { return 99; });
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 99);
}

TEST(Optional, ValueOrElse_HasValue) {
    std::optional<int> opt = 5;
    int result = cpx::value_or_else(opt, []() { return 99; });
    EXPECT_EQ(result, 5);
}

TEST(Optional, ValueOrElse_Empty) {
    std::optional<int> opt;
    int result = cpx::value_or_else(opt, []() { return 99; });
    EXPECT_EQ(result, 99);
}

// ─── Result<T, E> ─────────────────────────────────────────────────────────────

TEST(Result, OkIsOk) {
    auto r = Result<int>::ok(42);
    EXPECT_TRUE(r.is_ok());
    EXPECT_FALSE(r.is_err());
    EXPECT_TRUE(bool(r));
    EXPECT_EQ(*r, 42);
    EXPECT_EQ(r.value(), 42);
}

TEST(Result, ErrIsErr) {
    auto r = Result<int, std::string>::err("oops");
    EXPECT_FALSE(r.is_ok());
    EXPECT_TRUE(r.is_err());
    EXPECT_FALSE(bool(r));
    EXPECT_EQ(r.error(), "oops");
}

TEST(Result, ValueOrDefault) {
    auto ok  = Result<int>::ok(42);
    auto err = Result<int>::err("fail");
    EXPECT_EQ(ok.value_or(0), 42);
    EXPECT_EQ(err.value_or(0), 0);
}

TEST(Result, ValueThrowsOnErr) {
    auto r = Result<int>::err("bad");
    EXPECT_THROW(r.value(), cpx::bad_result_access);
}

TEST(Result, ErrorThrowsOnOk) {
    auto r = Result<int>::ok(1);
    EXPECT_THROW(r.error(), cpx::bad_result_access);
}

TEST(Result, AndThen_Ok) {
    auto r = Result<int>::ok(21)
                 .and_then([](int n) -> Result<int> { return Result<int>::ok(n * 2); });
    ASSERT_TRUE(r.is_ok());
    EXPECT_EQ(*r, 42);
}

TEST(Result, AndThen_Err) {
    auto r = Result<int>::err("fail")
                 .and_then([](int n) -> Result<int> { return Result<int>::ok(n * 2); });
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(r.error(), "fail");
}

TEST(Result, Transform_Ok) {
    auto r = Result<int>::ok(21).transform([](int n) { return n * 2; });
    ASSERT_TRUE(r.is_ok());
    EXPECT_EQ(*r, 42);
}

TEST(Result, Transform_Err) {
    auto r = Result<int>::err("oops").transform([](int n) { return n * 2; });
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(r.error(), "oops");
}

TEST(Result, Transform_TypeChange) {
    auto r = Result<int>::ok(42).transform([](int n) { return std::to_string(n); });
    ASSERT_TRUE(r.is_ok());
    EXPECT_EQ(*r, "42");
}

TEST(Result, MapErr) {
    auto r = Result<int>::err("bad").map_err([](const std::string &e) { return e.size(); });
    ASSERT_TRUE(r.is_err());
    EXPECT_EQ(r.error(), 3u);
}

TEST(Result, OrElse_Err) {
    auto r = Result<int>::err("fail")
                 .or_else([](const std::string &) -> Result<int> { return Result<int>::ok(99); });
    ASSERT_TRUE(r.is_ok());
    EXPECT_EQ(*r, 99);
}

TEST(Result, OrElse_Ok) {
    auto r = Result<int>::ok(7)
                 .or_else([](const std::string &) -> Result<int> { return Result<int>::ok(99); });
    ASSERT_TRUE(r.is_ok());
    EXPECT_EQ(*r, 7);
}

// ─── overload / visit ─────────────────────────────────────────────────────────

TEST(Overload, VisitInt) {
    std::variant<int, float, std::string> v = 42;
    int got = 0;
    cpx::visit(v,
        [&](int i)         { got = i; },
        [&](float)         { got = -1; },
        [&](std::string)   { got = -2; }
    );
    EXPECT_EQ(got, 42);
}

TEST(Overload, VisitString) {
    std::variant<int, float, std::string> v = std::string("hello");
    std::string got;
    cpx::visit(v,
        [&](int)           { got = "int"; },
        [&](float)         { got = "float"; },
        [&](std::string s) { got = s; }
    );
    EXPECT_EQ(got, "hello");
}

TEST(Overload, DirectOverload) {
    auto handler = cpx::overload{
        [](int i) { return i * 2; },
        [](double d) { return static_cast<int>(d * 10); },
    };
    EXPECT_EQ(handler(5), 10);
    EXPECT_EQ(handler(3.5), 35);
}

// ─── iter collect ─────────────────────────────────────────────────────────────

TEST(Iter, CollectVector) {
    std::vector<int> src = {1, 2, 3, 4, 5};
    auto collected = cpx::iterate(src).collect<std::vector>();
    EXPECT_EQ(collected, src);
}

TEST(Iter, CollectAfterDrop) {
    std::vector<int> src = {10, 20, 30, 40, 50};
    auto collected = cpx::iterate(src).drop(2).collect<std::vector>();
    std::vector<int> expected = {30, 40, 50};
    EXPECT_EQ(collected, expected);
}

TEST(Iter, CollectAfterTake) {
    std::vector<int> src = {1, 2, 3, 4, 5};
    auto collected = cpx::iterate(src).take(3).collect<std::vector>();
    std::vector<int> expected = {1, 2, 3};
    EXPECT_EQ(collected, expected);
}
