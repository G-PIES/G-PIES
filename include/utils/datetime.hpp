#ifndef DATETIME_HPP
#define DATETIME_HPP

#include <ctime>
#include <iostream>
#include <iterator>
#include <locale>

namespace datetime
{
    static void utc_now(std::string& str)
    {
        std::time_t time = std::time(nullptr);
        char datetime_str[21] = "yyyy-mm-ddThh:mm:ssZ";
        std::strftime(datetime_str, 21, "%F %T", std::gmtime(&time));
        str = std::string(datetime_str);
    }
}

#endif // DATETIME_HPP