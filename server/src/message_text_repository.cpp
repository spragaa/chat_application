#include "message_text_repository.hpp"
#include <regex>

namespace server {

MessageTextRepository::MessageTextRepository(const std::string& connection_string) {
	try {
		DEBUG_MSG("[MessageTextRepository::MessageTextRepository] Parsing message_text database connection string: " + connection_string);
		auto connection_options = parse_config_string(connection_string);

		// debug - don't log password in production!
		// i will...
		DEBUG_MSG("[MessageTextRepository::MessageTextRepository] Parsed options - Host: " + connection_options.host +
		          ", Port: " + std::to_string(connection_options.port) +
		          ", User: " + connection_options.user +
		          ", Password: " + connection_options.password +
		          ", DB: " + std::to_string(connection_options.db));
		_redis = std::make_unique<sw::redis::Redis>(connection_options);
		// auto ping_result = _redis->ping();
		// DEBUG_MSG("[MessageTextRepository::MessageTextRepository] PING response: " + ping_result);
		INFO_MSG("[MessageTextRepository::MessageTextRepository] Successfully connected to message_text Redis database");
	}
	catch (const std::exception& e) {
		ERROR_MSG("[MessageTextRepository::MessageTextRepository] " +  std::string(e.what()));
		FATAL_MSG("[MessageTextRepository::MessageTextRepository] Failed to connect to Redis database");
		throw;
	}
}

sw::redis::ConnectionOptions MessageTextRepository::parse_config_string(const std::string& connection_string) {
	sw::redis::ConnectionOptions options;
	std::regex connection_regex("redis://(?:(\\w+):)?([^@]+)@([\\w.]+):(\\d+)(?:/(\\d+))?");
	std::smatch matches;
	if (std::regex_match(connection_string, matches, connection_regex)) {
		if (matches[1].matched) {
			options.user = matches[1].str();
		}
		options.password = matches[2].str();
		options.host = matches[3].str();
		options.port = std::stoi(matches[4].str());
		if (matches[5].matched) {
			options.db = std::stoi(matches[5].str());
		}
	} else {
		throw std::invalid_argument("[MessageTextRepository::parse_config_string] Invalid connection string format");
	}
	DEBUG_MSG("[MessageTextRepository::parseConnectionString] Parsed successfully!");
	return options;
}

int MessageTextRepository::create(const common::MessageText& msg) {
	try {
		std::string value = msg.get_text();
		INFO_MSG("[MessageTextRepository::create] Attempting to insert value: " + value);

		long long new_id = _redis->incr("message_id_counter");
		std::string key = std::to_string(new_id);

		_redis->set(key, value);
		auto result = _redis->get(key);
		if (!result) {
			ERROR_MSG("[MessageTextRepository::create] Verification failed. Unable to retrieve set value");
			return 0;
		}

		DEBUG_MSG("[MessageTextRepository::create] MessageText created successfully: " + value + " with id: " + key);
		return static_cast<int>(new_id);
	} catch(const sw::redis::Error& e) {
		ERROR_MSG("[MessageTextRepository::create] " + std::string(e.what()));
		return 0;
	} catch(const std::exception& e) {
		ERROR_MSG("[MessageTextRepository::create] " + std::string(e.what()));
		return 0;
	}
}

std::optional<common::MessageText> MessageTextRepository::read(int id) {
	try {
		auto result = _redis->get(std::to_string(id));
		if (result) {
			INFO_MSG("[MessageTextRepository::read] Successfully read message with id: " + std::to_string(id));
			return common::MessageText(id, *result);
		} else {
			WARN_MSG("[MessageTextRepository::read] No message found with id: " + std::to_string(id));
			return std::nullopt;
		}
	} catch (const sw::redis::Error& e) {
		ERROR_MSG("[MessageTextRepository::read] Redis error: " + std::string(e.what()));
		return std::nullopt;
	} catch (const std::exception& e) {
		ERROR_MSG("[MessageTextRepository::read] " + std::string(e.what()));
		return std::nullopt;
	}
}

bool MessageTextRepository::update(const common::MessageText& message) {
	try {
		auto key = std::to_string(message.get_id());
		auto exists = _redis->exists(key);
		if (exists) {
			_redis->set(key, message.get_text());
			INFO_MSG("[MessageTextRepository::update] Successfully updated message with id: " + key);
			return true;
		} else {
			WARN_MSG("[MessageTextRepository::update] No message found with id: " + key);
			return false;
		}
	} catch (const sw::redis::Error& e) {
		ERROR_MSG("[MessageTextRepository::update] Redis error: " + std::string(e.what()));
		return false;
	} catch (const std::exception& e) {
		ERROR_MSG("[MessageTextRepository::update] " + std::string(e.what()));
		return false;
	}
}

bool MessageTextRepository::remove(int id) {
	try {
		auto key = std::to_string(id);
		auto removed = _redis->del(key);
		if (removed > 0) {
			INFO_MSG("[MessageTextRepository::remove] Successfully removed message with id: " + key);
			return true;
		} else {
			WARN_MSG("[MessageTextRepository::remove] No message found with id: " + key);
			return false;
		}
	} catch (const sw::redis::Error& e) {
		ERROR_MSG("[MessageTextRepository::remove] Redis error: " + std::string(e.what()));
		return false;
	} catch (const std::exception& e) {
		ERROR_MSG("[MessageTextRepository::remove] " + std::string(e.what()));
		return false;
	}
}

}