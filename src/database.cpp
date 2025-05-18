#include "database.h"
#include <string>
#include <fstream>

#include "helpers.h"


auto Table::addColumn(std::string const& n, ColumnType const& t, bool const& pk, bool const& nn) const -> void {
    columns.emplace_back(n, t, pk, nn);
}

auto Table::commit() const -> void {
    auto tableFile = std::ofstream(name + ".txt");
    tableFile << "Table:\n";
    for (auto const &column: columns) {
        tableFile << column.name << " " << columnTypeToString(column.type) << " " << column.primaryKey << " " << column.notNull << std::endl;
    }
    tableFile << "Data:\n";
    tableFile.close();
}