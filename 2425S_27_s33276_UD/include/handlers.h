#pragma once
#include <string>

auto createTableHandler(std::string const &query) -> void;

auto alterTableHandler(std::string const &query) -> void;

auto dropTableHandler(std::string const &query) -> void;

auto selectHandler(std::string const &query) -> void;

auto updateHandler(std::string const &query) -> void;

auto deleteHandler(std::string const &query) -> void;

auto insertIntoHandler(std::string const &query) -> void;

