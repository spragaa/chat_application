#pragma once

#include "base_repository.hpp"
#include "user.hpp"
#include "db_manager.hpp"
#include <optional>
#include <nlohmann/json.hpp>

class UserRepository : public BaseRepository<User> {
public:
    UserRepository(DBManager& db_manager, const std::string& connection_name);
    virtual ~UserRepository() override;

    int create(const User& user) override;
    std::optional<User> read(int id) override;
    bool update(const User& user) override;
    bool remove(int id) override;
    
    bool authorize(int user_id, const std::string& nickname, const std::string& password);

    // std::optional<User> findByNickname(const std::string& nickname);

private:
    User construct_user(const nlohmann::json& user_json);
    nlohmann::json pqxx_result_to_json(const pqxx::result& r) const;
    
private:
    // move inside base repo
    std::string connection_name;
};