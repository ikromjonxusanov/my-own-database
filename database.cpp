#include <string>
#include <utility>
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
    UUID,
};

class Column {
    std::string name;
    ColumnType type;
public:
    Column(std::string const& n, ColumnType const& t) : name(n), type(t) {}
};

class Table {
    std::string name;
    std::vector<Column> columns;
public:
    Table(std::string  n) : name(n) {}

    auto add_column(std::string n, ColumnType t) -> void {
        columns.push_back(Column(n, t));
    }
};
