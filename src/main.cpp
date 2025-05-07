#include <fmt/base.h>
#include <fmt/ranges.h>
#include <iostream>
#include <string>
#include <vector>
#include "database.h"

auto toLowerCase(std::string& str) -> void {
    for (auto& c : str) {
        c = std::tolower(c);
    }
}


auto execute(std::vector<std::string> const& query) -> void {
    fmt::println("{}", query);
    if (query.size() >= 3) {
        if (query[0] == "create" && query[1] == "table") {
            auto t1 = Table(query[2]);
            t1.createTableFile();
        }
    }
}
auto main() -> int {
    fmt::print("HELP; EXIT;\n>> ");
    auto query = std::vector<std::string>();

    while (true) {
        auto input = std::string();
        std::cin >> input;
        toLowerCase(input);

        if (input.back() == ';') {
            query.push_back(input);
            execute(query);
            query.clear();
            fmt::print(">> ");
        }
        query.push_back(input);

        if (input == "help;") {
            fmt::println("Helper\n1. CREATE TABLE <table_name>;\n2. DROP TABLE <table_name>");
        }
        if (input == "exit;") {
            return 0;
        }
    }
}