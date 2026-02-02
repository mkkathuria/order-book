#pragma once

#include <string>
#include <sstream>
#include <iomanip>

inline std::string format_usd(double amount) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << amount;
    std::string s = oss.str();

    auto dot             = s.find('.');
    std::string int_part = s.substr(0, dot);
    std::string dec_part = s.substr(dot);

    // Insert commas right-to-left into the integer part
    std::string formatted;
    int count = 0;
    for (int i = static_cast<int>(int_part.size()) - 1; i >= 0; --i) {
        if (count > 0 && count % 3 == 0)
            formatted = "," + formatted;
        formatted = int_part[i] + formatted;
        ++count;
    }

    return "$" + formatted + dec_part;
}
