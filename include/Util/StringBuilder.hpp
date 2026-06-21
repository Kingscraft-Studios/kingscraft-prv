#pragma once
#include <sstream>
#include <string>

namespace lve {

class StringBuilder {
public:
    template<typename... Args>
    static std::string build(Args... args) {
        std::stringstream ss;

        (ss << ... << args);

        return ss.str();
    }
};

}
