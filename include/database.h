#pragma once


enum ColumnType {
    INT,
    FLOAT,
    STRING,
    BOOL,
    DATE,
    TIME,
    DATETIME,
    TIMESTAMP,
};

class Column {
    std::string name;
    ColumnType type;
    bool primaryKey = false;
    bool notNull = true;

public:
    Column(
        std::string const &n, ColumnType const &t,
        bool const &pk, bool const &nn
    ) : name(n), type(t), primaryKey(pk), notNull(nn) {}

    ~Column() = default;

    auto getName() const;

    auto getType() const;

    auto isPrimaryKey() const;

    auto isNotNull() const;

};


class Table {
    std::string name;
    mutable std::vector<Column> columns = std::vector<Column>();

public:
    Table(std::string const &n) : name(n) {}

    Table(std::string const &n, std::vector<Column> const &columns) : name(n), columns(columns) {}

    ~Table() = default;

    auto add_column(Column const &column) const;

    void createTableFile() const;
};
