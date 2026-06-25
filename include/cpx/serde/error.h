#ifndef CPX_SERDE_ERROR_H
#define CPX_SERDE_ERROR_H

#include "cpx/nomodule.h"
#include <exception>
#include <string>

namespace cpx::serde {
    CPX_EXPORT class error : public std::exception {
    protected:
        mutable std::string what_;

    public:
        std::string context;
        std::string msg;
        std::string path;

        explicit error(std::string msg)
            : msg(std::move(msg)) {}

        error(std::string_view key, std::string msg)
            : context("." + std::string(key))
            , msg(std::move(msg)) {}

        error(size_t key, std::string msg)
            : context("[" + std::to_string(key) + "]")
            , msg(std::move(msg)) {}

        error &add_context(std::string_view key) & {
            context = "." + std::string(key) + std::move(context);
            return *this;
        }

        error &add_context(size_t key) & {
            context = "[" + std::to_string(key) + "]" + std::move(context);
            return *this;
        }

        const char *what() const noexcept override {
            std::string root = !path.empty() ? "<" + path + ">" : context.empty() ? "<root>" : "";
            what_            = "Error at " + root + context + ": " + msg;
            return what_.c_str();
        }
    };

    CPX_EXPORT class type_mismatch_error : public error {
    public:
        std::string expected_type;

        type_mismatch_error(const std::string &expected_type, const std::string &got, const std::string &extra = "")
            : error("Type mismatch error: expect `" + expected_type + "` got `" + got + "`" + (extra.empty() ? "" : ": ") + extra)
            , expected_type(expected_type) {}
    };

    CPX_EXPORT class size_mismatch_error : public error {
    public:
        size_mismatch_error(size_t expect, size_t got)
            : error("Size mismatch error: expect `" + std::to_string(expect) + "` got `" + std::to_string(got) + "`") {}
    };

    CPX_EXPORT class duplicate_oneof_error : public error {
    public:
        explicit duplicate_oneof_error(std::string_view oneof_signature)
            : error("multiple oneof fields set: `" + std::string(oneof_signature) + "`") {}
    };
} // namespace cpx::serde

#endif
