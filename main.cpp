#include <fmt/base.h>
#include <iostream>
#include <string>


auto main() -> int {
    fmt::print("HELP; EXIT;\n>> ");
    while (true) {
        auto input = std::string();
        std::cin >> input;
        if (input == "HELP;") {
            fmt::println("Helper\n1. CREATE TABLE <table_name>;\n2. DROP TABLE <table_name>");
        }
        fmt::print("\n>> ");
        if (input == "EXIT;") {
            return 0;
        }
    }
}