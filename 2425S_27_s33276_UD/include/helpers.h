#pragma once
#include <iostream>

#include "database.h"

auto toLowerCase(std::string const &str) -> std::string;

auto mapType(const std::string &typeStr) -> ColumnType;

auto columnTypeToString(ColumnType const &type) -> std::string;

auto checkCreateTable(std::string const &query) -> bool;

auto checkAlterTable(std::string const &query) -> bool;

auto checkDropTable(std::string const &query) -> bool;

auto checkSelect(std::string const &query) -> bool;

auto checkInsertInto(std::string const &query) -> bool;

auto checkUpdate(std::string const &query) -> bool;

auto checkDelete(std::string const &query) -> bool;

auto checkTableExists(std::string const &tableName) -> bool;

auto validateDataType(const std::string &value, ColumnType type) -> bool;

auto getTypeString(ColumnType type) -> std::string;
