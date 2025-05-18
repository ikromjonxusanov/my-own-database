#include <filesystem>
#include <fstream>
#include <string>
#include <regex>
#include <fmt/base.h>
#include "database.h"
#include "helpers.h"
#include <sstream>

#include "fmt/xchar.h"

auto createTableHandler(std::string const &query) -> void {
    // check "create table" query syntax
    if (!checkCreateTable(query)) {
        fmt::println("Invalid CREATE TABLE syntax");
        return;
    }
    // get table name using finding first table then finding '(' where the table name is between them
    auto tStart = query.find("table") + 5;
    auto tEnd = query.find('(');
    auto tableName = query.substr(tStart, tEnd - tStart);

    // remove white space or white space at the end
    tableName = std::regex_replace(tableName, std::regex(R"(^\s+|\s+$)"), "");


    if (checkTableExists(tableName)) {
        fmt::print("Table '{}' already exists.\n", tableName);
        return;
    }

    // get columns definition
    auto colStart = query.find('(') + 1;
    auto colEnd = query.find_last_of(')');
    auto columnsStr = query.substr(colStart, colEnd - colStart);

    // https://www.geeksforgeeks.org/processing-strings-using-stdistringstream/
    // learning material string stream for splitting string

    auto columns = std::vector<Column>();
    auto ss = std::stringstream(columnsStr);
    auto colDef = std::string();

    while (std::getline(ss, colDef, ',')) {
        // remove white space or white space at the end
        colDef = std::regex_replace(colDef, std::regex(R"(^\s+|\s+$)"), "");
        auto colStream = std::istringstream(colDef);
        auto name = std::string();
        auto type = std::string();
        auto token = std::string();
        auto notNull = false, primaryKey = false;

        // Get column name and type
        colStream >> name >> type;

        // checking column is (NOT NULL or/and PRIMARY KEY)
        while (colStream >> token) {
            if (toLowerCase(token) == "not") {
                colStream >> token;
                if (toLowerCase(token) == "null") {
                    notNull = true;
                } else {
                    fmt::println("Invalid NOT NULL syntax");
                    return;
                }
            } else if (toLowerCase(token) == "primary") {
                colStream >> token;
                if (toLowerCase(token) == "key") {
                    primaryKey = true;
                } else {
                    fmt::println("Invalid PRIMARY KEY syntax");
                    return;
                }
            } else {
                fmt::println("Invalid syntax");
                return;
            }
        }

        // checking column name is valid
        auto columnType = mapType(type);
        if (columnType == ColumnType::UNKNOWN) {
            fmt::println("Unknown column '{}'", type);
            return;
        }
        columns.emplace_back(name, columnType, primaryKey, notNull);
    }
    auto table = Table(tableName, columns);
    table.commit();
    fmt::print("Table '{}' created successfully\n", tableName);
}

auto alterTableHandler(std::string const &query) -> void {
    if (checkAlterTable(query)) {
        fmt::println("Invalid ALTER TABLE syntax");
        return;
    }

    auto tableStart = query.find("table") + 5;
    auto actionStart = query.find("add");
    if (actionStart == std::string::npos) {
        actionStart = query.find("drop");
    }

    if (tableStart == std::string::npos || actionStart == std::string::npos) {
        fmt::println("Invalid ALTER TABLE syntax");
        return;
    }

    auto tableName = query.substr(tableStart, actionStart - tableStart);
    tableName = std::regex_replace(tableName, std::regex(R"(^\s+|\s+$)"), "");

    if (!checkTableExists(tableName)) {
        fmt::print("Table '{}' does not exist.\n", tableName);
        return;
    }

    auto filename = tableName + ".txt";
    auto readFile = std::ifstream(filename);
    if (!readFile.is_open()) {
        fmt::println("Failed to open table file.");
        return;
    }

    auto fileLines = std::vector<std::string>();
    auto columns = std::vector<Column>();
    auto line = std::string();
    auto isDataSection = false;
    auto data = std::vector<std::vector<std::string> >();

    while (std::getline(readFile, line)) {
        if (line == "Data:") {
            isDataSection = true;
            fileLines.push_back(line);
            continue;
        }

        if (!isDataSection) {
            if (line != "Table:") {
                auto iss = std::istringstream(line);
                auto name = std::string();
                auto type = std::string();
                auto primaryKey = false, notNull = false;
                iss >> name >> type >> primaryKey >> notNull;
                columns.emplace_back(name, mapType(type), primaryKey, notNull);
            }
            fileLines.push_back(line);
        } else {
            auto dataStream = std::istringstream(line);
            auto rowData = std::vector<std::string>();
            auto value = std::string();
            while (dataStream >> value) {
                rowData.push_back(value);
            }
            data.push_back(rowData);
        }
    }
    readFile.close();

    auto isAdd = query.find("add") != std::string::npos;
    auto columnPos = query.find("column");


    auto columnDetails = query.substr(columnPos + 6);
    columnDetails = std::regex_replace(columnDetails, std::regex(R"(^\s+|\s+$)"), "");

    if (isAdd) {
        auto dataStream = std::istringstream(columnDetails);
        auto name = std::string();
        auto type = std::string();
        auto token = std::string();
        auto notNull = false, primaryKey = false;

        dataStream >> name >> type;

        while (dataStream >> token) {
            if (toLowerCase(token) == "not") {
                dataStream >> token;
                if (toLowerCase(token) == "null") {
                    notNull = true;
                }
            } else if (toLowerCase(token) == "primary") {
                dataStream >> token;
                if (toLowerCase(token) == "key") {
                    primaryKey = true;
                }
            }
        }

        for (const auto &col: columns) {
            if (col.name == name) {
                fmt::println("Column '{}' already exists", name);
                return;
            }
        }

        auto columnType = mapType(type);
        if (columnType == ColumnType::UNKNOWN) {
            fmt::println("Unknown column type '{}'", type);
            return;
        }

        columns.emplace_back(name, columnType, primaryKey, notNull);

        for (auto &row: data) {
            row.push_back("NULL");
        }
    } else {
        auto columnName = columnDetails;
        auto columnIndex = -1;

        for (auto i = 0; i < columns.size(); ++i) {
            if (columns[i].name == columnName) {
                if (columns[i].primaryKey) {
                    fmt::println("Cannot drop primary key column");
                    return;
                }
                columnIndex = i;
                break;
            }
        }

        if (columnIndex == -1) {
            fmt::println("Column '{}' not found", columnName);
            return;
        }

        columns.erase(columns.begin() + columnIndex);

        for (auto &row: data) {
            row.erase(row.begin() + columnIndex);
        }
    }

    auto writeFile = std::ofstream(filename);
    if (!writeFile.is_open()) {
        fmt::println("Failed to open file for writing");
        return;
    }

    writeFile << "Table:\n";

    for (const auto &col: columns) {
        writeFile << col.name << " "
                << getTypeString(col.type) << " "
                << (col.primaryKey ? "1" : "0") << " "
                << (col.notNull ? "1" : "0") << "\n";
    }

    writeFile << "Data:\n";
    for (const auto &row: data) {
        for (auto i = 0; i < row.size(); ++i) {
            writeFile << row[i];
            if (i < row.size() - 1) {
                writeFile << " ";
            }
        }
        writeFile << "\n";
    }

    writeFile.close();
    fmt::println("Table altered successfully");
}

auto dropTableHandler(std::string const &query) -> void {
    // checking "drop table" query syntax
    if (!checkDropTable(query)) {
        fmt::println("Invalid DROP TABLE syntax");
        return;
    }

    // get table name
    auto tStart = query.find("table") + 5;
    auto tableName = query.substr(tStart);

    // remove white space, white space at the end and semicolon at the end
    tableName = std::regex_replace(tableName, std::regex(R"(^\s+|\s+$|;)"), "");
    if (!checkTableExists(tableName)) {
        fmt::print("Table '{}' does not exist.\n", tableName);
        return;
    }
    std::filesystem::path filePath = tableName + ".txt";
    if (std::filesystem::remove(filePath)) {
        fmt::println("Table '{}' deleted successfully.", tableName);
    } else {
        fmt::println("Failed to delete the table.");
    }
}

auto selectHandler(std::string const &query) -> void {
    if (!checkSelect(query)) {
        fmt::println("INVALID SELECT syntax");
        return;
    }

    auto fields = std::string();
    auto tableName = std::string();
    auto whereClause = std::string();

    auto fromPos = query.find("from");

    // get the fields (columns) from the query
    fields = query.substr(6, fromPos - 6);
    fields = std::regex_replace(fields, std::regex(R"(^\s+|\s+$)"), "");

    tableName = query.substr(fromPos + 4);
    auto wherePos = tableName.find("where");

    // change the name before "where"
    if (wherePos != std::string::npos) {
        tableName = tableName.substr(0, wherePos);
        whereClause = tableName.substr(wherePos + 5);
    }

    tableName = std::regex_replace(tableName, std::regex(R"(^\s+|\s+$)"), "");
    whereClause = std::regex_replace(whereClause, std::regex(R"(^\s+|\s+$)"), "");

    if (!checkTableExists(tableName)) {
        fmt::print("Table '{}' does not exist.\n", tableName);
        return;
    }

    auto filename = tableName + ".txt";
    auto file = std::ifstream(filename);

    auto line = std::string();
    auto allColumns = std::vector<Column>();

    // for skipping the first line which is "Table:"
    std::getline(file, line);

    auto name = std::string();
    auto type = std::string();
    auto primaryKey = bool();
    auto notNull = bool();

    // read until "Data:" and get all table's columns
    while (std::getline(file, line) && line != "Data:") {
        auto dataStream = std::istringstream(line);
        dataStream >> name >> type >> primaryKey >> notNull;
        allColumns.emplace_back(name, mapType(type), primaryKey, notNull);
    }

    auto selectedColumns = std::vector<size_t>();
    if (fields == "*") {
        for (auto i = 0; i < allColumns.size(); ++i) {
            selectedColumns.push_back(i);
        }
    } else {
        auto fieldStream = std::istringstream(fields);
        auto field = std::string();
        while (std::getline(fieldStream, field, ',')) {
            field = std::regex_replace(field, std::regex(R"(^\s+|\s+$)"), "");
            bool found = false;
            for (auto i = 0; i < allColumns.size(); ++i) {
                if (allColumns[i].name == field) {
                    selectedColumns.push_back(i);
                    found = true;
                    break;
                }
            }
            if (!found) {
                fmt::println("Column '{}' not found in table.", field);
                return;
            }
        }
    }

    // where clause
    auto whereColumn = std::string();
    auto whereOperator = std::string();
    auto whereValue = std::string();

    if (!whereClause.empty()) {
        auto whereStream = std::istringstream(whereClause);
        whereStream >> whereColumn;
        whereStream >> whereOperator;
        whereStream >> whereValue;

        // remove quotes from string values
        whereValue = std::regex_replace(whereValue, std::regex(R"(^'(.*)'$)"), "");
    }

    // print table columns
    for (auto idx: selectedColumns) {
        fmt::print("{:<15}", allColumns[idx].name);
    }
    fmt::print("\n");

    for (auto i = 0; i < selectedColumns.size(); ++i) {
        fmt::print("{:<15}", "---------------");
    }
    fmt::print("\n");

    // read and print data
    while (std::getline(file, line)) {
        auto dataStream = std::istringstream(line);
        std::vector<std::string> rowValues;
        std::string value;

        // read all values for the row
        while (dataStream >> value) {
            rowValues.push_back(value);
        }

        // where clause
        bool includeRow = true;
        if (!whereClause.empty()) {
            auto whereColIndex = 0;
            bool foundWhereColumn = false;

            for (auto i = 0; i < allColumns.size(); ++i) {
                if (allColumns[i].name == whereColumn) {
                    whereColIndex = i;
                    foundWhereColumn = true;
                    break;
                }
            }

            if (!foundWhereColumn) {
                fmt::println("In WHERE clause column '{}' not found.", whereColumn);
                return;
            }

            // https://www.geeksforgeeks.org/stdstoi-function-in-cpp/ learning resource where I learnt std::stoi
            if (whereOperator == "=") {
                includeRow = (rowValues[whereColIndex] == whereValue);
            } else if (whereOperator == ">") {
                includeRow = (std::stoi(rowValues[whereColIndex]) > std::stoi(whereValue));
            } else if (whereOperator == "<") {
                includeRow = (std::stoi(rowValues[whereColIndex]) < std::stoi(whereValue));
            }
        }

        // printing the row if it matches the clause
        if (includeRow) {
            for (auto idx: selectedColumns) {
                fmt::print("{:<15}", rowValues[idx]);
            }
            fmt::print("\n");
        }
    }
}

auto insertIntoHandler(std::string const &query) -> void {
    if (!checkInsertInto(query)) {
        fmt::println("Invalid insert into syntax");
        return;
    }
    auto tableStart = query.find("into") + 4;
    auto columnStart = query.find('(');

    auto tableName = query.substr(tableStart, columnStart - tableStart);
    tableName = std::regex_replace(tableName, std::regex(R"(^\s+|\s+$)"), "");

    // check existence
    if (!checkTableExists(tableName)) {
        fmt::print("Table '{}' does not exist.\n", tableName);
        return;
    }

    // read table columns
    auto filename = tableName + ".txt";
    auto readFile = std::ifstream(filename);

    // Read table columns
    auto line = std::string();
    auto tableColumns = std::vector<Column>();

    std::getline(readFile, line);

    while (std::getline(readFile, line) && line != "Data:") {
        auto dataStream = std::istringstream(line);
        auto name = std::string();
        auto type = std::string();
        auto primaryKey = false;
        auto notNull = false;
        dataStream >> name >> type >> primaryKey >> notNull;
        tableColumns.emplace_back(name, mapType(type), primaryKey, notNull);
    }

    // extract column names from query
    auto valuesStart = query.find("values");

    auto columnList = query.substr(columnStart + 1, query.find(')', columnStart) - columnStart - 1);
    auto insertColumns = std::vector<std::string>();
    auto columnStream = std::istringstream(columnList);
    auto col = std::string();

    while (std::getline(columnStream, col, ',')) {
        col = std::regex_replace(col, std::regex(R"(^\s+|\s+$)"), "");
        insertColumns.push_back(col);
    }

    auto valueListStart = query.find('(', valuesStart);
    auto valueListEnd = query.find(')', valueListStart);

    auto valueList = query.substr(valueListStart + 1, valueListEnd - valueListStart - 1);
    auto values = std::vector<std::string>();
    auto valueStream = std::istringstream(valueList);
    auto val = std::string();

    while (std::getline(valueStream, val, ',')) {
        val = std::regex_replace(val, std::regex(R"(^\s*'?\s*|\s*'?\s*$)"), "");
        values.push_back(val);
    }

    if (insertColumns.size() != values.size()) {
        fmt::println("Number of columns and values do not match");
        return;
    }

    // validate columns exist
    auto fullData = std::vector<std::string>(tableColumns.size());
    auto columnData = std::vector<bool>(tableColumns.size(), false);

    for (auto i = 0; i < insertColumns.size(); ++i) {
        bool columnFound = false;
        for (auto j = 0; j < tableColumns.size(); ++j) {
            if (tableColumns[j].name == insertColumns[i]) {
                // check data type
                if (!validateDataType(values[i], tableColumns[j].type)) {
                    fmt::println("Invalid data type for column '{}'", tableColumns[j].name);
                    return;
                }
                fullData[j] = values[i];
                columnData[j] = true;
                columnFound = true;
                break;
            }
        }
        if (!columnFound) {
            fmt::println("Column '{}' not found in table", insertColumns[i]);
            return;
        }
    }

    for (auto i = 0; i < tableColumns.size(); ++i) {
        if (!columnData[i] && tableColumns[i].notNull) {
            fmt::println("This column '{}' is not nullable you should type data", tableColumns[i].name);
            return;
        }
    }

    // check PRIMARY KEY
    if (std::filesystem::exists(filename)) {
        auto line = std::string();
        auto checkFile = std::ifstream(filename);
        while (std::getline(checkFile, line)) {
            if (line == "Data:") {
                while (std::getline(checkFile, line)) {
                    std::istringstream iss(line);
                    std::vector<std::string> rowValues;
                    std::string value;
                    while (iss >> value) {
                        rowValues.push_back(value);
                    }

                    // Check primary key constraints
                    for (auto i = 0; i < tableColumns.size(); ++i) {
                        if (tableColumns[i].primaryKey && columnData[i] &&
                            rowValues[i] == fullData[i]) {
                            fmt::println("PRIMARY KEY failed value '{}' exists", fullData[i]);
                            return;
                        }
                    }
                }
            }
        }
        checkFile.close();
    }

    auto writeFile = std::ofstream(filename, std::ios::app);

    for (auto i = 0; i < fullData.size(); ++i) {
        writeFile << (columnData[i] ? fullData[i] : "NULL");
        if (i < fullData.size() - 1) {
            writeFile << " ";
        }
    }
    writeFile << "\n";
    writeFile.close();

    fmt::println("Record added successfully");
}

auto updateHandler(std::string const &query) -> void {
    if (!checkUpdate(query)) {
        fmt::println("Invalid update syntax");
        return;
    }
    auto tableStart = query.find("update") + 6;
    auto setPos = query.find("set");
    if (setPos == std::string::npos) {
        fmt::println("Invalid UPDATE syntax: SET keyword missing");
        return;
    }

    auto tableName = query.substr(tableStart, setPos - tableStart);
    tableName = std::regex_replace(tableName, std::regex(R"(^\s+|\s+$)"), "");

    if (!checkTableExists(tableName)) {
        fmt::print("Table '{}' does not exist.\n", tableName);
        return;
    }

    // Parse SET clause and WHERE condition
    auto where = query.find("where");
    auto setClause = std::string();
    if (where != std::string::npos) {
        setClause = query.substr(setPos + 3, where - setPos - 3);
    } else {
        setClause = query.substr(setPos + 3);
    }
    setClause = std::regex_replace(setClause, std::regex(R"(^\s+|\s+$)"), "");

    auto whereClause = std::string();

    if (where != std::string::npos) {
        whereClause = query.substr(where + 5);
        whereClause = std::regex_replace(whereClause, std::regex(R"(^\s+|\s+$)"), "");
    }

    auto filename = tableName + ".txt";
    auto readFile = std::ifstream(filename);

    auto columns = std::vector<Column>();
    auto data = std::vector<std::vector<std::string> >();
    auto line = std::string();

    std::getline(readFile, line);

    while (std::getline(readFile, line) && line != "Data:") {
        auto dataStream = std::istringstream(line);
        auto name = std::string();
        auto type = std::string();
        auto primaryKey = false;
        auto notNull = false;
        dataStream >> name >> type >> primaryKey >> notNull;
        columns.emplace_back(name, mapType(type), primaryKey, notNull);
    }

    while (std::getline(readFile, line)) {
        auto dataStream = std::istringstream(line);
        auto rowData = std::vector<std::string>();
        auto value = std::string();
        while (dataStream >> value) {
            rowData.push_back(value);
        }
        data.push_back(rowData);
    }
    readFile.close();

    auto updates = std::map<size_t, std::string>();
    auto setStream = std::istringstream(setClause);
    auto logicalOperator = std::string();
    while (std::getline(setStream, logicalOperator, ',')) {
        logicalOperator = std::regex_replace(logicalOperator, std::regex(R"(^\s+|\s+$)"), "");
        auto equalsPos = logicalOperator.find('=');
        if (equalsPos == std::string::npos) continue;

        auto colName = logicalOperator.substr(0, equalsPos);
        auto newValue = logicalOperator.substr(equalsPos + 1);
        colName = std::regex_replace(colName, std::regex(R"(^\s+|\s+$)"), "");
        newValue = std::regex_replace(newValue, std::regex(R"(^\s*'?\s*|\s*'?\s*$)"), "");

        for (auto i = 0; i < columns.size(); ++i) {
            if (columns[i].name == colName) {
                if (!validateDataType(newValue, columns[i].type)) {
                    fmt::println("Invalid data type for column '{}'", colName);
                    return;
                }
                updates[i] = newValue;
                break;
            }
        }
    }

    auto whereColumn = std::string();
    auto whereOperator = std::string();
    auto whereValue = std::string();

    if (!whereClause.empty()) {
        auto whereStream = std::istringstream(whereClause);
        whereStream >> whereColumn >> whereOperator >> whereValue;
        whereValue = std::regex_replace(whereValue, std::regex(R"(^\s*'?\s*|\s*'?\s*$)"), "");
    }

    auto updateCount = 0;
    for (auto &row: data) {
        auto updateRow = whereClause.empty();
        if (!updateRow) {
            for (auto i = 0; i < columns.size(); ++i) {
                if (columns[i].name == whereColumn) {
                    if (whereOperator == "=") {
                        updateRow = (row[i] == whereValue);
                    } else if (whereOperator == ">") {
                        updateRow = (std::stoi(row[i]) > std::stoi(whereValue));
                    } else if (whereOperator == "<") {
                        updateRow = (std::stoi(row[i]) < std::stoi(whereValue));
                    }
                    break;
                }
            }
        }

        if (updateRow) {
            for (const auto &[index, value]: updates) {
                row[index] = value;
            }
            updateCount++;
        }
    }

    auto writeFile = std::ofstream(filename);

    writeFile << "Table:\n";
    for (const auto &col: columns) {
        writeFile << col.name << " "
                << getTypeString(col.type) << " "
                << (col.primaryKey ? "1" : "0") << " "
                << (col.notNull ? "1" : "0") << "\n";
    }

    writeFile << "Data:\n";
    for (const auto &row: data) {
        for (auto i = 0; i < row.size(); ++i) {
            writeFile << row[i];
            if (i < row.size() - 1) {
                writeFile << " ";
            }
        }
        writeFile << "\n";
    }

    writeFile.close();
    fmt::println("{} row(s) updated", updateCount);
}

auto deleteHandler(std::string const &query) -> void {
    if (!checkDelete(query)) {
        fmt::println("Invalid DELETE syntax");
        return;
    }
    auto fromPos = query.find("from");

    auto wherePos = query.find("where");
    auto tableName = std::string();
    if (wherePos != std::string::npos) {
        tableName = query.substr(fromPos + 4, wherePos - fromPos - 4);
    } else {
        tableName = query.substr(fromPos + 4);
    }

    tableName = std::regex_replace(tableName, std::regex(R"(^\s+|\s+$)"), "");

    if (!checkTableExists(tableName)) {
        fmt::print("Table '{}' does not exist.\n", tableName);
        return;
    }

    auto whereClause = std::string();
    if (wherePos != std::string::npos) {
        whereClause = query.substr(wherePos + 5);
        whereClause = std::regex_replace(whereClause, std::regex(R"(^\s+|\s+$)"), "");
    }

    auto filename = tableName + ".txt";
    auto readFile = std::ifstream(filename);

    auto columns = std::vector<Column>();
    auto data = std::vector<std::vector<std::string> >();
    auto line = std::string();

    std::getline(readFile, line);

    while (std::getline(readFile, line) && line != "Data:") {
        auto dataStream = std::istringstream(line);
        auto name = std::string();
        auto type = std::string();
        auto primaryKey = false;
        auto notNull = false;
        dataStream >> name >> type >> primaryKey >> notNull;
        columns.emplace_back(name, mapType(type), primaryKey, notNull);
    }

    while (std::getline(readFile, line)) {
        auto dataStream = std::istringstream(line);
        auto rowData = std::vector<std::string>();
        auto value = std::string();
        while (dataStream >> value) {
            rowData.push_back(value);
        }
        data.push_back(rowData);
    }
    readFile.close();

    auto whereColumn = std::string();
    auto whereOperator = std::string();
    auto whereValue = std::string();

    if (!whereClause.empty()) {
        auto whereStream = std::istringstream(whereClause);
        whereStream >> whereColumn >> whereOperator >> whereValue;
        whereValue = std::regex_replace(whereValue, std::regex(R"(^'|'$)"), "");
    }

    auto initialSize = data.size();
    if (!whereClause.empty()) {
        auto it = data.begin();
        while (it != data.end()) {
            auto deleteRow = false;
            for (auto i = 0; i < columns.size(); ++i) {
                if (columns[i].name == whereColumn) {
                    if (whereOperator == "=") {
                        deleteRow = ((*it)[i] == whereValue);
                    } else if (whereOperator == ">") {
                        deleteRow = (std::stoi((*it)[i]) > std::stoi(whereValue));
                    } else if (whereOperator == "<") {
                        deleteRow = (std::stoi((*it)[i]) < std::stoi(whereValue));
                    }
                    break;
                }
            }
            if (deleteRow) {
                it = data.erase(it);
            } else {
                ++it;
            }
        }
    } else {
        data.clear();
    }

    auto writeFile = std::ofstream(filename);

    writeFile << "Table:\n";
    for (const auto &col: columns) {
        writeFile << col.name << " "
                << getTypeString(col.type) << " "
                << (col.primaryKey ? "1" : "0") << " "
                << (col.notNull ? "1" : "0") << "\n";
    }

    writeFile << "Data:\n";
    for (const auto &row: data) {
        for (auto i = 0; i < row.size(); ++i) {
            writeFile << row[i];
            if (i < row.size() - 1) {
                writeFile << " ";
            }
        }
        writeFile << "\n";
    }

    writeFile.close();
    fmt::println("{} row(s) deleted", initialSize - data.size());
}
