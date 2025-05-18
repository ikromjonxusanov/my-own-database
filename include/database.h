#pragma once
#include <iostream>
#include <vector>

enum ColumnType {
    INT,
    FLOAT,
    STRING,
    BOOL,
    DATE,
    TIME,
    DATETIME,
    TIMESTAMP,
    UNKNOWN
};

struct Column {
    std::string name;
    ColumnType type;
    bool primaryKey = false;
    bool notNull = true;

    Column(
        std::string const &n, ColumnType const &t,
        bool const &pk, bool const &nn
    ) : name(n), type(t), primaryKey(pk), notNull(nn) {
    }

    ~Column() = default;
};


struct Table {
    std::string name;

    Table(std::string const &n) : name(n) {}

    Table(std::string const &n, std::vector<Column> const &columns) : name(n), columns(columns) {}

    ~Table() = default;

    auto addColumn(std::string const &n, ColumnType const &t, bool const &pk, bool const &nn) const -> void;
    auto commit() const -> void;

private:
    mutable std::vector<Column> columns = std::vector<Column>();
};
