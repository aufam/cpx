#ifndef CPX_TIME_H
#define CPX_TIME_H

#include <ctime>
#include <string>
#include <stdexcept>

namespace cpx {
    inline constexpr auto tm_min = []() {
        std::tm tm_min{};
        tm_min.tm_year = 70;
        tm_min.tm_mday = 1;
        return tm_min;
    }();

    inline constexpr auto tm_max = []() {
        std::tm tm_max{};
        tm_max.tm_year = 1100;
        tm_max.tm_mday = 1;
        return tm_max;
    }();

    inline std::string tm_to_string(const std::tm &tm) {
        std::string str(64, '\0');
        size_t      len = std::strftime(str.data(), str.size(), "%Y-%m-%dT%H:%M:%SZ", &tm);
        if (len == 0)
            throw std::runtime_error("Failed to serialize std::tm");

        str.resize(len);
        return str;
    }

    inline std::tm tm_from_string(const std::string &str, const std::tm *default_value = nullptr) {
        if (str.empty()) {
            if (default_value)
                return *default_value;
            throw std::runtime_error("Empty datetime string");
        }

        bool date_time = str.size() == 20 && str[4] == '-' && str[7] == '-' && str[10] == 'T' && str[13] == ':' && str[16] == ':' && str[19] == 'Z';
        bool date_only = str.size() == 10 && str[4] == '-' && str[7] == '-';
        if (!date_time && !date_only) {
            throw std::runtime_error("Invalid datetime format: " + str);
        }

        std::tm tm = {};
        try {
            tm.tm_year = std::stoi(str.substr(0, 4)) - 1900;
            tm.tm_mon  = std::stoi(str.substr(5, 2)) - 1;
            tm.tm_mday = std::stoi(str.substr(8, 2));
            if (!date_only) {
                tm.tm_hour = std::stoi(str.substr(11, 2));
                tm.tm_min  = std::stoi(str.substr(14, 2));
                tm.tm_sec  = std::stoi(str.substr(17, 2));
            }
            tm.tm_isdst = 0; // UTC
        } catch (...) {
            throw std::runtime_error("Invalid datetime format: " + str);
        }
        return tm;
    }

    inline std::tm tm_now() {
        time_t  t  = time(nullptr);
        std::tm tm = {};
#if defined(_WIN32)
        _gmtime64_s(&tm, &t);
#else
        ::gmtime_r(&t, &tm);
#endif
        return tm;
    }
} // namespace cpx

#endif
