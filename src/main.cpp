#include <filesystem>
#include <fstream>
#include <fmt/base.h>
#include <iostream>
#include <string>
#include <vector>
#include "helpers.h"



#include <iostream>
#include <regex>
#include <string>
#include "helpers.h"
#include "handlers.h"


auto execute(std::string const& input) {
    std::string query = toLowerCase(input);

    query = std::regex_replace(query, std::regex(R"(^\s+|\s+$|;$)"), "");

    if (query.starts_with("create table")) {
        createTableHandler(query);
    } else if (query.starts_with("alter table")) {
        alterTableHandler(query);
    } else if (query.starts_with("drop table")) {
        dropTableHandler(query);
    } else if (query.starts_with("select")) {
        selectHandler(query);
    } else if (query.starts_with("update")) {
        updateHandler(query);
    } else if (query.starts_with("delete")) {
        deleteHandler(query);
    } else if (query.starts_with("insert into")) {
        insertIntoHandler(query);
    } else {
        std::cout << "[ERROR] Unsupported or invalid SQL command.\n";
    }
}

auto main() -> int {
    fmt::print("HELP; EXIT;\n");
    auto query = std::vector<std::string>();

    while (true) {
        fmt::print(">> ");
        std::string input;
        std::getline(std::cin, input);

        if (input == "help;") {
            fmt::println("Helper\n1. CREATE TABLE <table_name>;\n2. DROP TABLE <table_name>");
        }
        else if (input == "exit;") {
            return 0;
        } else {
            execute(input);
        }



    }
}