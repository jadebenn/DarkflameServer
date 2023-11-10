#include "MySQLDatabase.h"

void MySQLDatabase::InsertNewBugReport(
	const std::string_view body,
	const std::string_view clientVersion,
	const std::string_view otherPlayer,
	const std::string_view selection,
	const uint32_t characterId) {
	ExecuteInsert("INSERT INTO `bug_reports`(body, client_version, other_player_id, selection, reporter_id) VALUES (?, ?, ?, ?, ?)",
		body, clientVersion, otherPlayer, selection, characterId);
}
