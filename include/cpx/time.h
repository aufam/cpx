#ifndef CPX_TIME_H
#define CPX_TIME_H

#include <cstdlib>
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
            throw std::runtime_error("Failed to stringify std::tm");

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

    inline std::string ts_to_string(const std::timespec &ts, bool iso = false) {
        constexpr auto ten_years = 24l * 3600 * 365;

        time_t    seconds     = ts.tv_sec;
        long long nanoseconds = ts.tv_nsec;
        seconds += nanoseconds / 1'000'000'000;
        nanoseconds %= 1'000'000'000;
        if (nanoseconds < 0) {
            nanoseconds += 1'000'000'000;
            --seconds;
        }

        // assume timestamp if it is more than 10 years
        if (seconds > ten_years || seconds < 0) {
            std::tm tm = *std::gmtime(&seconds);
            return tm_to_string(tm, ts.tv_nsec);
        }

        long long total_ns = std::llabs(seconds) * 1'000'000'000ll + std::llabs(nanoseconds);

        constexpr long long NS_PER_SEC  = 1'000'000'000ll;
        constexpr long long NS_PER_MIN  = 60ll * NS_PER_SEC;
        constexpr long long NS_PER_HOUR = 60ll * NS_PER_MIN;
        constexpr long long NS_PER_DAY  = 24ll * NS_PER_HOUR;

        long long days = total_ns / NS_PER_DAY;
        total_ns %= NS_PER_DAY;

        long long hours = total_ns / NS_PER_HOUR;
        total_ns %= NS_PER_HOUR;

        long long minutes = total_ns / NS_PER_MIN;
        total_ns %= NS_PER_MIN;

        long long secs  = total_ns / NS_PER_SEC;
        long long nanos = total_ns % NS_PER_SEC;

        std::string str;

        bool negative = (seconds < 0 || nanoseconds < 0);
        if (negative)
            str += "-";

        if (iso) {
            str += "P";

            if (days)
                str += std::to_string(days) + "D";

            bool has_time = hours || minutes || secs || nanos;

            if (has_time || days == 0) {
                str += "T";

                if (hours)
                    str += std::to_string(hours) + "H";

                if (minutes)
                    str += std::to_string(minutes) + "M";

                if (nanos) {
                    // fractional seconds
                    char buf[32];
                    std::snprintf(buf, sizeof(buf), "%lld.%09lld", secs, nanos);

                    std::string frac = buf;

                    // trim trailing zeros
                    frac.erase(frac.find_last_not_of('0') + 1);

                    // remove dangling '.'
                    if (frac.back() == '.')
                        frac.pop_back();

                    str += frac + "S";
                } else if (secs || (!hours && !minutes)) {
                    str += std::to_string(secs) + "S";
                }
            }
        } else {
            if (days)
                str += std::to_string(days) + "d";

            if (hours)
                str += std::to_string(hours) + "h";

            if (minutes)
                str += std::to_string(minutes) + "m";

            if (secs)
                str += std::to_string(secs) + "s";

            auto millis = nanos / 1'000'000;
            if (millis)
                str += std::to_string(millis) + "ms";

            if (str.empty() || str == "-")
                str += "0ms";
        }

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
        if (pos < str.size() && str[pos] == 'P') {
            ++pos;
            bool in_time = false;
            found        = true;

            while (pos < str.size()) {
                if (str[pos] == 'T') {
                    in_time = true;
                    ++pos;
                    continue;
                }

                if (!std::isdigit(static_cast<unsigned char>(str[pos])))
                    throw std::runtime_error("Expected digit in ISO duration");

                // parse integer part
                long long value = 0;

                while (pos < str.size() && std::isdigit(static_cast<unsigned char>(str[pos]))) {
                    value = value * 10 + (str[pos] - '0');
                    ++pos;
                }

                // fractional seconds
                if (pos < str.size() && str[pos] == '.') {
                    if (pos + 1 >= str.size())
                        throw std::runtime_error("Invalid fractional seconds");

                    ++pos;

                    std::string frac;

                    while (pos < str.size() && std::isdigit(static_cast<unsigned char>(str[pos]))) {
                        frac += str[pos++];
                    }

                    if (pos >= str.size() || str[pos] != 'S')
                        throw std::runtime_error("Fraction only allowed on seconds");

                    while (frac.size() < 9)
                        frac.push_back('0');

                    if (frac.size() > 9)
                        frac.resize(9);

                    total_seconds += value;
                    total_nanos += std::stoll(frac);

                    ++pos; // S
                    continue;
                }

                if (pos >= str.size())
                    throw std::runtime_error("Missing ISO duration unit");

                switch (str[pos]) {
                case 'D':
                    total_seconds += value * 86400ll;
                    break;

                case 'H':
                    total_seconds += value * 3600ll;
                    break;

                case 'M':
                    if (!in_time)
                        throw std::runtime_error("Months not supported in ISO duration");
                    total_seconds += value * 60ll;
                    break;

                case 'S':
                    total_seconds += value;
                    break;

                default:
                    throw std::runtime_error("Unsupported ISO duration unit");
                }

                ++pos;
            }
        } else {
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

                if (str.compare(pos, 2, "ms") == 0) {
                    total_nanos += value * 1'000'000ll;
                    pos += 2;
                } else if (str[pos] == 'd') {
                    total_seconds += value * 24ll * 3600ll;
                    ++pos;
                } else if (str[pos] == 'h') {
                    total_seconds += value * 3600ll;
                    ++pos;
                } else if (str[pos] == 'm') {
                    total_seconds += value * 60ll;
                    ++pos;
                } else if (str[pos] == 's') {
                    total_seconds += value;
                    ++pos;
                } else {
                    throw std::runtime_error("Unknown duration unit");
                }
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
