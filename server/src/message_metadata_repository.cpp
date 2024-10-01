#include "message_metadata_repository.hpp"

MessageMetadataRepository::MessageMetadataRepository(DBManager& db_manager, const std::string& connection_name) : BaseRepository(db_manager), connection_name(connection_name) {
	DEBUG_MSG("MessageMetadataRepository created");
}

MessageMetadataRepository::~MessageMetadataRepository() = default;

int MessageMetadataRepository::create(const Message& message) {
	try {
		pqxx::work txn(db_manager.get_connection(connection_name));

		std::stringstream ss;
		auto time_point = message.get_created_timestamp();
		auto time_t = std::chrono::system_clock::to_time_t(time_point);
		ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
		std::string formatted_time = ss.str();

		pqxx::result r = txn.exec_params(
			"INSERT INTO messages (sender_id, receiver_id, text, created_timestamp) "
			"VALUES ($1, $2, $3, $4) RETURNING id",
			message.get_sender_id(),
			message.get_receiver_id(),
			message.get_text(),
			formatted_time
			);
		txn.commit();

		if (r.empty()) {
			DEBUG_MSG("[MessageMetadataRepository::create] Failed to insert message");
			return 0;
		}

		int inserted_id = r[0][0].as<int>();
		DEBUG_MSG("[MessageMetadataRepository::create] Message inserted successfully with id: " + std::to_string(inserted_id));
		return inserted_id;
	} catch (const std::exception& e) {
		DEBUG_MSG("[MessageMetadataRepository::create] Exception caught: " + std::string(e.what()));
		return 0;
	}
}

std::optional<Message> MessageMetadataRepository::read(int id) {
	try {
		pqxx::work txn(db_manager.get_connection(connection_name));
		pqxx::result r = txn.exec_params("SELECT * FROM messages WHERE id = $1", id);
		if (r.empty()) {
			DEBUG_MSG("[MessageMetadataRepository::read] No message found with id: " + std::to_string(id));
			return std::nullopt;
		}
		DEBUG_MSG("[MessageMetadataRepository::read] Message found with id: " + std::to_string(id));
		return construct_message(r[0]);
	} catch (const std::exception& e) {
		DEBUG_MSG("[MessageMetadataRepository::read] Exception caught: " + std::string(e.what()));
		return std::nullopt;
	}
}

bool MessageMetadataRepository::update(const Message& message) {
	try {
		pqxx::work txn(db_manager.get_connection(connection_name));

		std::stringstream ss;
		auto time_point = message.get_last_edited_timestamp().value_or(std::chrono::system_clock::now());
		auto time_t = std::chrono::system_clock::to_time_t(time_point);
		ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
		std::string formatted_time = ss.str();

		pqxx::result r = txn.exec_params(
			"UPDATE messages SET text = $1, last_edited_timestamp = $2 "
			"WHERE id = $3",
			message.get_text(),
			formatted_time,
			message.get_id()
			);
		txn.commit();

		if (r.affected_rows() == 0) {
			DEBUG_MSG("[MessageMetadataRepository::update] No message updated");
			return false;
		}

		DEBUG_MSG("[MessageMetadataRepository::update] Message updated successfully with id: " + std::to_string(message.get_id()));
		return true;
	} catch (const std::exception& e) {
		DEBUG_MSG("[MessageMetadataRepository::update] Exception caught: " + std::string(e.what()));
		return false;
	}
}

bool MessageMetadataRepository::remove(int id) {
	try {
		pqxx::work txn(db_manager.get_connection(connection_name));

		std::stringstream ss;
		auto time_point = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(time_point);
		ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
		std::string formatted_time = ss.str();

		pqxx::result r = txn.exec_params(
			"UPDATE messages SET deleted = TRUE, deleted_timestamp = $1 "
			"WHERE id = $2",
			formatted_time,
			id
			);
		txn.commit();

		if (r.affected_rows() == 0) {
			DEBUG_MSG("[MessageMetadataRepository::remove] No message deleted");
			return false;
		}

		DEBUG_MSG("[MessageMetadataRepository::remove] Message deleted successfully with id: " + std::to_string(id));
		return true;
	} catch (const std::exception& e) {
		DEBUG_MSG("[MessageMetadataRepository::remove] Exception caught: " + std::string(e.what()));
		return false;
	}
}

std::chrono::system_clock::time_point parse_timestamp(const std::string& timestamp_str) {
	std::tm tm = {};
	std::stringstream ss(timestamp_str);
	ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
	return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

Message MessageMetadataRepository::construct_message(const pqxx::row& row) {
	Message msg(
		row["sender_id"].as<int>(),
		row["receiver_id"].as<int>(),
		row["text"].as<std::string>()
		);
	msg.set_id(row["id"].as<int>());
	msg.set_deleted(row["deleted"].as<bool>());
	msg.set_created_timestamp(parse_timestamp(row["created_timestamp"].as<std::string>()));
	if (!row["deleted_timestamp"].is_null())
		msg.set_deleted_timestamp(parse_timestamp(row["deleted_timestamp"].as<std::string>()));
	if (!row["last_edited_timestamp"].is_null())
		msg.set_last_edited_timestamp(parse_timestamp(row["last_edited_timestamp"].as<std::string>()));
	return msg;
}