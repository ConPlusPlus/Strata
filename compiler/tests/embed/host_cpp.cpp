// host_cpp.cpp - the C++ wrapper (strata.hpp)
#include <iostream>
#include "strata.hpp"
int main() {
    std::cout << "version: " << (strata::version().empty() ? "FAILED" : "ok") << "\n";
    std::cout << "check: " << (strata::check("prog/main.strata") ? "ok" : "FAILED") << "\n";
    strata::check_source("prog/main.strata", "print(nope)\n");
    std::string d = strata::diagnostics();
    strata::reset();                        // std::string copies survive the reset
    std::cout << d;
    std::string c = strata::emit("prog/main.strata");
    std::cout << "emit: " << (c.find("int main(") != std::string::npos ? "ok" : "FAILED") << "\n";
    return 0;
}
