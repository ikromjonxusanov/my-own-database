#include "database.h"
#include <string>
#include <iostream>
#include <fstream>

#include "helpers.h"


auto Table::addColumn(std::string const &n, ColumnType const &t, bool const &pk, bool const &nn) const -> void {
    columns.emplace_back(n, t, pk, nn);
}

auto Table::commit() const -> void {
    // Create and open a text file
    std::ofstream tableFile(name + ".txt");

    // Write to the file
    tableFile << "Table:\n";
    for (auto const &column: columns) {
        tableFile << column.name << " " << columnTypeToString(column.type) << " " << column.primaryKey << " " << column.notNull << std::endl;
    }
    tableFile << "Data:\n";
    tableFile.close();
    //
    // std::string myText;
    //
    // // Read from the text file
    // std::ifstream MyReadFile("db.sqlite3");
    //
    // // Use a while loop together with the getline() function to read the file line by line
    // while (getline(MyReadFile, myText)) {
    //     // Output the text from the file
    //     std::cout << myText << std::endl;
    // }
    //
    // // Close the file
    // MyReadFile.close();
}