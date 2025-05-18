#include <filesystem>
#include <iostream>
#include "database.h"
#include <string>
#include <regex>


auto toLowerCase(std::string const &str) -> std::string {
    auto result = std::string();
    for (const auto &c: str) {
        result += std::tolower(c);
    }
    return result;
}


auto mapType(const std::string &typeStr) -> ColumnType {
    if (typeStr == "int") return INT;
    if (typeStr == "float") return FLOAT;
    if (typeStr.starts_with("varchar") || typeStr.starts_with("string")) return STRING;
    if (typeStr == "bool") return BOOL;
    if (typeStr == "date") return DATE;
    if (typeStr == "time") return TIME;
    if (typeStr == "datetime") return DATETIME;
    return UNKNOWN;
}

auto columnTypeToString(ColumnType const &type) -> std::string {
    switch (type) {
        case INT:
            return "int";
        case FLOAT:
            return "float";
        case STRING:
            return "string";
        case BOOL:
            return "bool";
        case DATE:
            return "date";
        case TIME:
            return "time";
        case DATETIME:
            return "datetime";
        default: return "unknown";
    }
}

auto checkCreateTable(std::string const &query) -> bool {
    std::regex pattern(
        "^CREATE\\s+TABLE\\s+\\w+\\s*\\(\\s*"
        "(?:\\w+\\s+[\\w\\(\\)]+\\s*(?:,\\s*)?)+"
        "\\s*\\)\\s*;?\\s*$",
        std::regex_constants::icase
    );
    return regex_match(query, pattern);
}

auto checkAlterTable(const std::string &query) -> bool {
    std::regex pattern(
        R"(^\s*ALTER\s+TABLE\s+\w+\s+(ADD|DROP)\s+COLUMN\s+\w+(\s+\w+)*\s*;?\s*$)",
        std::regex_constants::icase
    );
    return std::regex_match(query, pattern);
}

auto checkDropTable(std::string const &query) -> bool {
    std::regex pattern(
        "^DROP\\s+TABLE\\s+\\w+\\s*;?\\s*$",
        std::regex_constants::icase
    );
    return regex_match(query, pattern);
}

auto checkSelect(std::string const &query) -> bool {
    std::regex pattern(
        R"(^\s*SELECT\s+([\w\*,\s]+)\s+FROM\s+\w+(\s+WHERE\s+.+)?\s*;?\s*$)",
        std::regex_constants::icase
    );
    return std::regex_match(query, pattern);
}

auto checkInsertInto(std::string const &query) -> bool {
    std::regex pattern(
        R"(^\s*INSERT\s+INTO\s+\w+\s*\(([\w\s,]+)\)\s+VALUES\s*\(([^)]+)\)\s*;?\s*$)",
        std::regex_constants::icase
    );
    return std::regex_match(query, pattern);
}

auto checkUpdate(std::string const &query) -> bool {
    std::regex pattern(
        R"(^\s*UPDATE\s+\w+\s+SET\s+[\w\s=,'-]+(\s+WHERE\s+[\w\s=><'-]+)?\s*;?\s*$)",
        std::regex_constants::icase
    );
    return std::regex_match(query, pattern);
}

auto checkDelete(std::string const &query) -> bool {
    std::regex pattern(
        R"(^\s*DELETE\s+FROM\s+\w+(\s+WHERE\s+[\w\s=><'-]+)?\s*;?\s*$)",
        std::regex_constants::icase
    );
    return std::regex_match(query, pattern);
}

auto checkTableExists(std::string const &tableName) -> bool {
    std::filesystem::path filePath = tableName + ".txt";
    return std::filesystem::exists(filePath);
}

auto validateDataType(const std::string &value, ColumnType type) -> bool {
    try {
        switch (type) {
            case INT:
                std::stoi(value);
                return true;
            case FLOAT:
                std::stof(value);
                return true;
            case STRING:
                return true;
            case BOOL:
                return value == "true" || value == "false" || value == "0" || value == "1";
            // Add more type validations as needed
            default:
                return false;
        }
    } catch (...) {
        return false;
    }
}

auto getTypeString(ColumnType type) -> std::string {
    switch (type) {
        case INT: return "int";
        case FLOAT: return "float";
        case STRING: return "string";
        case BOOL: return "bool";
        case DATE: return "date";
        case TIME: return "time";
        case DATETIME: return "datetime";
        case TIMESTAMP: return "timestamp";
        default: return "unknown";
    }
}
