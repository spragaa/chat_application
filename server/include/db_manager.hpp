#pragma once

#include "debug.hpp"

#include <pqxx/pqxx>
#include <string>
#include <memory>
#include <unordered_map>

// rename to postgre db manager
class DBManager {
public:
	DBManager() = default;
	~DBManager() = default;

	void add_connection(const std::string& name, const std::string& connection_string);
	pqxx::connection& get_connection(const std::string& name);

private:
	std::unordered_map<std::string, std::unique_ptr<pqxx::connection> > connections;
};