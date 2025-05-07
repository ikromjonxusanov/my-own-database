#include <string>
#include <vector>
#include <fmt/base.h>
#include <iostream>
#include <fstream>
#include "database.h"


auto Column::getName() const { return name; }
auto Column::getType() const { return type; }
auto Column::isPrimaryKey() const { return primaryKey; }
auto Column::isNotNull() const { return notNull; }


auto Table::add_column(Column const &column) const {
    // Table cannot have more than one primary key
    if (column.isPrimaryKey()) {
        for (const Column &c: columns) {
            if (c.isPrimaryKey()) {
                fmt::println("This table already have a primary key!!");
                return;
            }
        }
    }
    columns.push_back(column);
}

void Table::createTableFile() const {
    std::ofstream tableFile(name + ".txt");
    tableFile.close();
    fmt::print("File created.\n");
}
