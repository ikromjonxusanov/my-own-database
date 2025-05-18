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
        std::cout << "Unsupported or invalid SQL command.\n";
    }
}

void helpHandler() {
    fmt::println("Help!");
    fmt::println("Supported commands:");
    fmt::println("  CREATE TABLE <table> (col1 type [PRIMARY KEY] [NOT NULL], ...);");
    fmt::println("  DROP TABLE <table>;");
    fmt::println("  ALTER TABLE <table> DROP COLUMN <col>;");
    fmt::println("  INSERT INTO <table> (col1, col2, ...) VALUES (val1, val2, ...);");
    fmt::println("  UPDATE <table> SET col1=val1, ... [WHERE col=val];");
    fmt::println("  DELETE FROM <table> [WHERE col=val];");
    fmt::println("  SELECT col1, col2 FROM <table> [WHERE col=val];");
}

auto main() -> int {
    fmt::print("HELP; EXIT;\n");
    auto query = std::vector<std::string>();

    while (true) {
        fmt::print(">> ");
        std::string input;
        std::getline(std::cin, input);

        if (input == "help;") {
            helpHandler();
        }
        else if (input == "exit;") {
            return 0;
        } else {
            execute(input);
        }



    }
}