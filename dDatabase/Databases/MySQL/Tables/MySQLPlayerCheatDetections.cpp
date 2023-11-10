#include "MySQLDatabase.h"

void MySQLDatabase::InsertCheatDetection(
	std::optional<uint32_t> userId,
	const std::string_view username,
	const std::string_view systemAddress,
	const std::string_view extraMessage) {
	ExecuteInsert(
		"INSERT INTO player_cheat_detections (account_id, name, violation_msg, violation_system_address) VALUES (?, ?, ?, ?)",
		userId, username, extraMessage, systemAddress);
}
