#ifndef CPX_TIME_H
#define CPX_TIME_H

#include <ctime>
#include <string>
#include <stdexcept>

namespace cpx {
    template <typename T>
    struct is_time : std::false_type {};

    template <>
    struct is_time<std::tm> : std::true_type {};

    template <>
    struct is_time<std::timespec> : std::true_type {};

    template <typename T>
    inline constexpr bool is_time_v = is_time<T>::value;

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


    inline std::string tm_to_string(const std::tm &tm, long long nanos = 0, int offset_mins = 0) {
        std::string str(64, '\0');
        size_t      len;
        const bool  time_only = tm.tm_mday == 0; // assume time only

        if (time_only) {
            len = std::strftime(str.data(), str.size(), "%H:%M:%S", &tm);
        } else {
            len = std::strftime(str.data(), str.size(), "%Y-%m-%dT%H:%M:%S", &tm);
        }

        if (len == 0)
            throw std::runtime_error("Failed to serialize std::tm");

        str.resize(len);

        if (nanos != 0) {
            long long ms = nanos / 1'000'000;
            char      frac[8];
            std::snprintf(frac, sizeof(frac), ".%03hd", (int16_t)ms);
            str += frac;
        }

        if (time_only)
            return str;

        // UTC
        if (offset_mins == 0) {
            str += 'Z';
        } else {
            char sign     = offset_mins < 0 ? '-' : '+';
            int  abs_mins = std::abs(offset_mins);
            int  hours    = abs_mins / 60;
            int  mins     = abs_mins % 60;

            char offset[9];
            std::snprintf(offset, sizeof(offset), "%c%02d:%02d", sign, (char)hours, (char)mins);

            str += offset;
        }
        return str;
    }

    inline std::string ts_to_string(const std::timespec &ts) {
        constexpr auto ten_years = 24l * 3600 * 365;

        time_t    seconds     = ts.tv_sec;
        long long nanoseconds = ts.tv_nsec;
        seconds += nanoseconds / 1'000'000'000;
        nanoseconds %= 1'000'000'000;
        if (nanoseconds < 0) {
            nanoseconds += 1'000'000'000;
            --seconds;
        }

        if (seconds > ten_years || seconds < 0) {
            std::tm tm = *std::gmtime(&seconds);
            return tm_to_string(tm, ts.tv_nsec);
        }

        long long total_ns = std::llabs(seconds) * 1'000'000'000ll + std::llabs(nanoseconds);

        long long days = total_ns / (24ll * 3600 * 1'000'000'000ll);
        total_ns %= (24ll * 3600 * 1'000'000'000ll);

        long long hours = total_ns / (3600ll * 1'000'000'000ll);
        total_ns %= (3600ll * 1'000'000'000ll);

        long long minutes = total_ns / (60ll * 1'000'000'000ll);
        total_ns %= (60ll * 1'000'000'000ll);

        long long secs = total_ns / 1'000'000'000ll;
        total_ns %= 1'000'000'000ll;

        long long ms = total_ns / 1'000'000ll;

        std::string str;

        if (days)
            str += std::to_string(days) + "d";
        if (hours)
            str += std::to_string(hours) + "h";
        if (minutes)
            str += std::to_string(minutes) + "m";
        if (secs)
            str += std::to_string(secs) + "s";
        if (ms)
            str += std::to_string(ms) + "ms";

        if (str.empty() || str == "-")
            str += "0s";

        return str;
    }

    inline std::tm tm_from_string(
        const std::string &str, const std::tm *default_value = nullptr, decltype(std::timespec::tv_nsec) *nanos = nullptr
    ) {
        if (nanos)
            *nanos = 0;

        if (str.empty()) {
            if (default_value)
                return *default_value;
            throw std::runtime_error("Empty datetime string");
        }

        bool date_only = str.size() == 10 && str[4] == '-' && str[7] == '-';
        bool has_time  = str.size() >= 20 && str[4] == '-' && str[7] == '-' && str[10] == 'T' && str[13] == ':' && str[16] == ':';
        if (!date_only && !has_time)
            throw std::runtime_error("Invalid datetime format: " + str);

        std::tm tm = {};
        try {
            tm.tm_year = std::stoi(str.substr(0, 4)) - 1900;
            tm.tm_mon  = std::stoi(str.substr(5, 2)) - 1;
            tm.tm_mday = std::stoi(str.substr(8, 2));

            if (has_time) {
                tm.tm_hour = std::stoi(str.substr(11, 2));
                tm.tm_min  = std::stoi(str.substr(14, 2));
                tm.tm_sec  = std::stoi(str.substr(17, 2));
                size_t pos = 19;

                if (pos < str.size() && str[pos] == '.') {
                    ++pos;
                    size_t frac_begin = pos;
                    while (pos < str.size() && std::isdigit(static_cast<unsigned char>(str[pos])))
                        ++pos;

                    std::string frac = str.substr(frac_begin, pos - frac_begin);
                    if (frac.empty() || frac.size() > 9)
                        throw std::runtime_error("Invalid fractional seconds");

                    // scale to nanoseconds
                    while (frac.size() < 9)
                        frac += '0';

                    if (nanos)
                        *nanos = std::stoll(frac);
                }

                // require UTC Z
                if (pos >= str.size() || str[pos] != 'Z')
                    throw std::runtime_error("Expected UTC suffix 'Z'");

                ++pos;
                if (pos != str.size())
                    throw std::runtime_error("Unexpected trailing characters");
            }

            tm.tm_isdst = 0; // UTC
        } catch (...) {
            throw std::runtime_error("Invalid datetime format: " + str);
        }

        return tm;
    }

    inline std::timespec ts_from_string(const std::string &str, const std::timespec *default_value = nullptr) {
        if (str.empty()) {
            if (default_value)
                return *default_value;
            throw std::runtime_error("Empty datetime string");
        }

        try {
            decltype(std::timespec::tv_nsec) ns;
            std::tm                          tm = tm_from_string(str, nullptr, &ns);
#ifdef _WIN32
            time_t sec = _mkgmtime(&tm);
#else
            time_t sec = timegm(&tm);
#endif
            return {sec, ns};
        } catch (std::runtime_error &e) {
            std::ignore = e;
        }

        size_t pos      = 0;
        bool   negative = false;

        if (str[pos] == '-') {
            negative = true;
            ++pos;
        }

        long long total_seconds = 0;
        long long total_nanos   = 0;

        bool found = false;

        while (pos < str.size()) {
            if (!std::isdigit(static_cast<unsigned char>(str[pos])))
                throw std::runtime_error("Expected digit in duration");

            found = true;

            // parse number
            long long value = 0;

            while (pos < str.size() && std::isdigit(static_cast<unsigned char>(str[pos]))) {
                value = value * 10 + (str[pos] - '0');
                ++pos;
            }

            // parse unit
            if (pos >= str.size())
                throw std::runtime_error("Missing duration unit");

            // ms
            if (str.compare(pos, 2, "ms") == 0) {
                total_nanos += value * 1'000'000ll;
                pos += 2;
            }
            // days
            else if (str[pos] == 'd') {
                total_seconds += value * 24ll * 3600ll;
                ++pos;
            }
            // hours
            else if (str[pos] == 'h') {
                total_seconds += value * 3600ll;
                ++pos;
            }
            // minutes
            else if (str[pos] == 'm') {
                total_seconds += value * 60ll;
                ++pos;
            }
            // seconds
            else if (str[pos] == 's') {
                total_seconds += value;
                ++pos;
            } else {
                throw std::runtime_error("Unknown duration unit");
            }
        }

        // normalize
        total_seconds += total_nanos / 1'000'000'000ll;
        total_nanos %= 1'000'000'000ll;

        if (negative) {
            total_seconds = -total_seconds;
            total_nanos   = -total_nanos;
        }

        if (!found)
            throw std::runtime_error("Invalid duration string");

        return {total_seconds, total_nanos};
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

    inline std::timespec ts_now() {
        std::timespec ts;
        timespec_get(&ts, TIME_UTC);
        return ts;
    }
} // namespace cpx

#endif
