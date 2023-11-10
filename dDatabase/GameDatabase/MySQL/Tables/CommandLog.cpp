#include "MySQLDatabase.h"

void MySQLDatabase::InsertSlashCommandUsage(const std::string_view command, const uint32_t characterId) {
	ExecuteInsert("INSERT INTO command_log (character_id, command) VALUES (?, ?);", characterId, command);
}
