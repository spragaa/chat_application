#pragma once

#include "common.hpp"

#include <string>
#include <set>
#include <optional>
#include <nlohmann/json.hpp>

// class Media;

class MessageMetadata {
public:

	MessageMetadata(
		const int sender_id,
		const int receiver_id
		);

	bool                    is_deleted() const noexcept;
	int                     get_id() const noexcept;
	int                     get_sender_id() const noexcept;
	int                     get_receiver_id() const noexcept;
	Timestamp               get_created_timestamp() const noexcept;
	std::optional<Timestamp>get_deleted_timestamp() const noexcept;
	std::optional<Timestamp>get_last_edited_timestamp() const noexcept;

	void set_id(const int& msg_id) noexcept;
	void set_sender(const int& t) noexcept;
	void set_created_timestamp(const Timestamp& timestamp) noexcept;
	void set_last_edited_timestamp(const Timestamp& timestamp) noexcept;
	void set_deleted_timestamp(const Timestamp& timestamp) noexcept;
	void set_deleted(const bool& d) noexcept;

	nlohmann::json to_json() const;
	MessageMetadata from_json(const nlohmann::json& j);

private:

	int id;
	int sender_id;
	int receiver_id;
	bool deleted;
	Timestamp created_timestamp;
	std::optional<Timestamp>deleted_timestamp;
	std::optional<Timestamp>last_edited_timestamp;
};
