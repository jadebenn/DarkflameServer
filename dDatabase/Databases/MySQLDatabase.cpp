#include "MySQLDatabase.h"

#include "Database.h"
#include "Game.h"
#include "dConfig.h"
#include "Logger.h"

using namespace DatabaseStructs;

namespace {
	sql::Driver* driver;
	sql::Connection* con;
	sql::Properties properties;
	std::string databaseName;
};

void MySQLDatabase::Connect() {
	driver = sql::mariadb::get_driver_instance();

	// The mariadb connector is *supposed* to handle unix:// and pipe:// prefixes to hostName, but there are bugs where
	// 1) it tries to parse a database from the connection string (like in tcp://localhost:3001/darkflame) based on the
	//    presence of a /
	// 2) even avoiding that, the connector still assumes you're connecting with a tcp socket
	// So, what we do in the presence of a unix socket or pipe is to set the hostname to the protocol and localhost,
	// which avoids parsing errors while still ensuring the correct connection type is used, and then setting the appropriate
	// property manually (which the URL parsing fails to do)
	const std::string UNIX_PROTO = "unix://";
	const std::string PIPE_PROTO = "pipe://";
	std::string mysql_host = Game::config->GetValue("mysql_host");
	if (mysql_host.find(UNIX_PROTO) == 0) {
		properties["hostName"] = "unix://localhost";
		properties["localSocket"] = mysql_host.substr(UNIX_PROTO.length()).c_str();
	} else if (mysql_host.find(PIPE_PROTO) == 0) {
		properties["hostName"] = "pipe://localhost";
		properties["pipe"] = mysql_host.substr(PIPE_PROTO.length()).c_str();
	} else {
		properties["hostName"] = mysql_host.c_str();
	}
	properties["user"] = Game::config->GetValue("mysql_username").c_str();
	properties["password"] = Game::config->GetValue("mysql_password").c_str();
	properties["autoReconnect"] = "true";

	databaseName = Game::config->GetValue("mysql_database").c_str();

	// `connect(const Properties& props)` segfaults in windows debug, but
	// `connect(const SQLString& host, const SQLString& user, const SQLString& pwd)` doesn't handle pipes/unix sockets correctly
	if (properties.find("localSocket") != properties.end() || properties.find("pipe") != properties.end()) {
		con = driver->connect(properties);
	} else {
		con = driver->connect(properties["hostName"].c_str(), properties["user"].c_str(), properties["password"].c_str());
	}
	con->setSchema(databaseName.c_str());
}

void MySQLDatabase::Destroy(std::string source, bool log) {
	if (!con) return;

	if (log) {
		if (source != "") LOG("Destroying MySQL connection from %s!", source.c_str());
		else LOG("Destroying MySQL connection!");
	}

	con->close();
	delete con;
}

void MySQLDatabase::ExecuteCustomQuery(const std::string_view query) {
	std::unique_ptr<sql::Statement>(con->createStatement())->execute(query.data());
}

sql::PreparedStatement* MySQLDatabase::CreatePreppedStmt(const std::string& query) {
	if (!con) {
		Connect();
		LOG("Trying to reconnect to MySQL");
	}

	if (!con->isValid() || con->isClosed()) {
		delete con;

		con = nullptr;

		Connect();
		LOG("Trying to reconnect to MySQL from invalid or closed connection");
	}

	return con->prepareStatement(sql::SQLString(query.c_str(), query.length()));
}

void MySQLDatabase::Commit() {
	con->commit();
}

bool MySQLDatabase::GetAutoCommit() {
	// TODO This should not just access a pointer.  A future PR should update this
	// to check for null and throw an error if the connection is not valid.
	return con->getAutoCommit();
}

void MySQLDatabase::SetAutoCommit(bool value) {
	// TODO This should not just access a pointer.  A future PR should update this
	// to check for null and throw an error if the connection is not valid.
	con->setAutoCommit(value);
}

// activity_log table

void MySQLDatabase::UpdateActivityLog(const uint32_t accountId, const eActivityType activityType, const LWOMAPID mapId) {
	ExecuteInsert("INSERT INTO activity_log (character_id, activity, time, map_id) VALUES (?, ?, ?, ?);",
		accountId, static_cast<uint32_t>(activityType), static_cast<uint32_t>(time(NULL)), mapId);
}

// accounts table

std::optional<AccountInfo> MySQLDatabase::GetAccountInfo(const std::string_view username) {
	auto result = ExecuteSelect("SELECT id, password, banned, locked, play_key_id, gm_level FROM accounts WHERE name = ? LIMIT 1;", username);

	if (!result->next()) {
		return std::nullopt;
	}

	AccountInfo toReturn;
	toReturn.id = result->getUInt("id");
	toReturn.maxGmLevel = static_cast<eGameMasterLevel>(result->getInt("gm_level"));
	toReturn.bcryptPassword = result->getString("password").c_str();
	toReturn.banned = result->getBoolean("banned");
	toReturn.locked = result->getBoolean("locked");
	toReturn.playKeyId = result->getUInt("play_key_id");

	return toReturn;
}

void MySQLDatabase::UpdateAccountUnmuteTime(const uint32_t accountId, const uint64_t timeToUnmute) {
	ExecuteUpdate("UPDATE accounts SET mute_expire = ? WHERE id = ?;", timeToUnmute, accountId);
}

void MySQLDatabase::UpdateAccountBan(const uint32_t accountId, const bool banned) {
	ExecuteUpdate("UPDATE accounts SET banned = ? WHERE id = ?;", banned, accountId);
}

void MySQLDatabase::UpdateAccountPassword(const std::string_view bcryptpassword, const uint32_t accountId) {
	ExecuteUpdate("UPDATE accounts SET password = ? WHERE id = ?;", bcryptpassword, accountId);
}

void MySQLDatabase::InsertNewAccount(const std::string_view username, const std::string_view bcryptpassword) {
	ExecuteInsert("INSERT INTO accounts (name, password, gm_level) VALUES (?, ?, ?);", username, bcryptpassword, static_cast<int32_t>(eGameMasterLevel::OPERATOR));
}

// charInfo table

std::optional<ApprovedNames> MySQLDatabase::GetApprovedCharacterNames() {
	auto result = ExecuteSelect("SELECT name FROM charinfo;");

	ApprovedNames toReturn;
	if (!result->next()) return std::nullopt;

	do {
		toReturn.names.push_back(result->getString("name").c_str());
	} while (result->next());

	return toReturn;
}

std::optional<uint32_t> MySQLDatabase::DoesCharacterExist(const std::string_view name) {
	auto result = ExecuteSelect("SELECT id FROM charinfo WHERE name = ? LIMIT 1;", name);

	if (!result->next()) {
		return std::nullopt;
	}
	return result->getUInt("id");
}

std::optional<uint32_t> MySQLDatabase::GetCharacterIdFromCharacterName(const std::string_view name) {
	auto result = ExecuteSelect("SELECT id FROM charinfo WHERE name = ? LIMIT 1;", name);

	if (!result->next()) {
		return std::nullopt;
	}

	return result->getUInt(1);
}

std::optional<CharacterInfo> CharInfoFromQueryResult(std::unique_ptr<sql::ResultSet> stmt) {
	if (!stmt->next()) {
		return std::nullopt;
	}

	CharacterInfo toReturn;

	toReturn.id = stmt->getUInt("id");
	toReturn.name = stmt->getString("name").c_str();
	toReturn.pendingName = stmt->getString("pending_name").c_str();
	toReturn.needsRename = stmt->getBoolean("needs_rename");
	toReturn.cloneId = stmt->getUInt64("prop_clone_id");
	toReturn.accountId = stmt->getUInt("account_id");
	toReturn.permissionMap = static_cast<ePermissionMap>(stmt->getUInt("permission_map"));

	return toReturn;
}

std::optional<CharacterInfo> MySQLDatabase::GetCharacterInfo(const uint32_t charId) {
	return CharInfoFromQueryResult(
		ExecuteSelect("SELECT name, pending_name, needs_rename, prop_clone_id, permission_map, id, account_id FROM charinfo WHERE id = ? LIMIT 1;", charId)
	);
}

std::optional<CharacterInfo> MySQLDatabase::GetCharacterInfo(const std::string_view name) {
	return CharInfoFromQueryResult(
		ExecuteSelect("SELECT name, pending_name, needs_rename, prop_clone_id, permission_map, id, account_id FROM charinfo WHERE name = ? LIMIT 1;", name)
	);
}

std::vector<uint32_t> MySQLDatabase::GetCharacterIds(const uint32_t accountId) {
	auto result = ExecuteSelect("SELECT id FROM charinfo WHERE account_id = ? ORDER BY last_login DESC LIMIT 4;", accountId);

	std::vector<uint32_t> toReturn;
	toReturn.reserve(result->rowsCount());
	while (result->next()) {
		toReturn.push_back(result->getUInt("id"));
	}

	return toReturn;
}

void MySQLDatabase::InsertNewCharacter(const uint32_t accountId, const uint32_t characterId, const std::string_view name, const std::string_view pendingName) {
	ExecuteInsert(
		"INSERT INTO `charinfo`(`id`, `account_id`, `name`, `pending_name`, `needs_rename`, `last_login`) VALUES (?,?,?,?,?,?)",
		characterId,
		accountId,
		name,
		pendingName,
		false,
		time(NULL));
}

void MySQLDatabase::SetCharacterName(const uint32_t characterId, const std::string_view name) {
	ExecuteUpdate("UPDATE charinfo SET name = ?, pending_name = '', needs_rename = 0, last_login = ? WHERE id = ? LIMIT 1;", name, time(NULL), characterId);
}

void MySQLDatabase::SetPendingCharacterName(const uint32_t characterId, const std::string_view name) {
	ExecuteUpdate("UPDATE charinfo SET pending_name = ?, needs_rename = 0, last_login = ? WHERE id = ? LIMIT 1", name, time(NULL), characterId);
}

void MySQLDatabase::UpdateLastLoggedInCharacter(const uint32_t characterId) {
	ExecuteUpdate("UPDATE charinfo SET last_login = ? WHERE id = ? LIMIT 1", time(NULL), characterId);
}

// friends table

std::optional<FriendsList> MySQLDatabase::GetFriendsList(const uint32_t charId) {
	auto friendsList = ExecuteSelect(
		R"QUERY(
			SELECT fr.requested_player, best_friend, ci.name FROM 
			(
				SELECT CASE 
				WHEN player_id = ? THEN friend_id 
				WHEN friend_id = ? THEN player_id 
				END AS requested_player, best_friend FROM friends
			) AS fr 
			JOIN charinfo AS ci ON ci.id = fr.requested_player 
			WHERE fr.requested_player IS NOT NULL AND fr.requested_player != ?;
		)QUERY", charId, charId, charId);

	if (!friendsList->next()) {
		return std::nullopt;
	}

	FriendsList toReturn;

	toReturn.friends.reserve(friendsList->rowsCount());

	do {
		FriendData fd;
		fd.friendID = friendsList->getUInt(1);
		fd.isBestFriend = friendsList->getInt(2) == 3; // 0 = friends, 1 = left_requested, 2 = right_requested, 3 = both_accepted - are now bffs
		fd.friendName = friendsList->getString(3).c_str();

		toReturn.friends.push_back(fd);
	} while (friendsList->next());
	return toReturn;
}

std::optional<BestFriendStatus> MySQLDatabase::GetBestFriendStatus(const uint32_t playerAccountId, const uint32_t friendAccountId) {
	auto result = ExecuteSelect("SELECT * FROM friends WHERE (player_id = ? AND friend_id = ?) OR (player_id = ? AND friend_id = ?) LIMIT 1;",
		playerAccountId,
		friendAccountId,
		friendAccountId,
		playerAccountId
	);

	if (!result->next()) {
		return std::nullopt;
	}

	BestFriendStatus toReturn;
	toReturn.playerAccountId = result->getUInt("player_id");
	toReturn.friendAccountId = result->getUInt("friend_id");
	toReturn.bestFriendStatus = result->getUInt("best_friend");

	return toReturn;
}

void MySQLDatabase::SetBestFriendStatus(const uint32_t playerAccountId, const uint32_t friendAccountId, const uint32_t bestFriendStatus) {
	ExecuteUpdate("UPDATE friends SET best_friend = ? WHERE (player_id = ? AND friend_id = ?) OR (player_id = ? AND friend_id = ?) LIMIT 1;",
		bestFriendStatus,
		playerAccountId,
		friendAccountId,
		friendAccountId,
		playerAccountId
	);
}

void MySQLDatabase::AddFriend(const uint32_t playerAccountId, const uint32_t friendAccountId) {
	ExecuteInsert("INSERT IGNORE INTO friends (player_id, friend_id, best_friend) VALUES (?, ?, 0);", playerAccountId, friendAccountId);
}

void MySQLDatabase::RemoveFriend(const uint32_t playerAccountId, const uint32_t friendAccountId) {
	ExecuteDelete("DELETE FROM friends WHERE (player_id = ? AND friend_id = ?) OR (player_id = ? AND friend_id = ?) LIMIT 1;",
		playerAccountId,
		friendAccountId,
		friendAccountId,
		playerAccountId
	);
}

// ugc table

void MySQLDatabase::RemoveUnreferencedUgcModels() {
	ExecuteDelete("DELETE FROM ugc WHERE id NOT IN (SELECT ugc_id FROM properties_contents WHERE ugc_id IS NOT NULL);");
}

void MySQLDatabase::InsertNewUgcModel(
	std::istringstream& sd0Data, // cant be const sad
	const uint32_t blueprintId,
	const uint32_t accountId,
	const uint32_t characterId) {
	const std::istream stream(sd0Data.rdbuf());
	ExecuteInsert(
		"INSERT INTO `ugc`(`id`, `account_id`, `character_id`, `is_optimized`, `lxfml`, `bake_ao`, `filename`) VALUES (?,?,?,?,?,?,?)",
		blueprintId,
		accountId,
		characterId,
		0,
		&stream,
		false,
		"weedeater.lxfml"
	);
}

std::vector<UgcModel> MySQLDatabase::GetAllUgcModels(const LWOOBJID& propertyId) {
	auto result = ExecuteSelect(
		"SELECT lxfml, u.id FROM ugc AS u JOIN properties_contents AS pc ON u.id = pc.ugc_id WHERE lot = 14 AND property_id = ? AND pc.ugc_id IS NOT NULL;",
		propertyId);

	std::vector<UgcModel> toReturn;

	while (result->next()) {
		UgcModel model;

		// blob is owned by the query, so we need to do a deep copy :/
		std::unique_ptr<std::istream> blob(result->getBlob("lxfml"));
		model.lxfmlData << blob->rdbuf();
		model.id = result->getUInt64("id");
		toReturn.push_back(std::move(model));
	}

	return toReturn; // move elision
}

void MySQLDatabase::DeleteUgcModelData(const LWOOBJID& modelId) {
	ExecuteDelete("DELETE FROM ugc WHERE id = ?;", modelId);
	ExecuteDelete("DELETE FROM properties_contents WHERE ugc_id = ?;", modelId);
}

void MySQLDatabase::UpdateUgcModelData(const LWOOBJID& modelId, std::istringstream& lxfml) {
	const std::istream stream(lxfml.rdbuf());
	auto update = ExecuteUpdate("UPDATE ugc SET lxfml = ? WHERE id = ?;", &stream, modelId);
}

std::vector<UgcModel> MySQLDatabase::GetUgcModels() {
	auto result = ExecuteSelect("SELECT id, lxfml FROM ugc;");

	std::vector<UgcModel> models;
	models.reserve(result->rowsCount());
	while (result->next()) {
		UgcModel model;
		model.id = result->getInt64("id");

		// blob is owned by the query, so we need to do a deep copy :/
		std::unique_ptr<std::istream> blob(result->getBlob("lxfml"));
		model.lxfmlData << blob->rdbuf();
		models.push_back(std::move(model));
	}

	return models;
}

// migration_history table

void MySQLDatabase::CreateMigrationHistoryTable() {
	ExecuteInsert("CREATE TABLE IF NOT EXISTS migration_history (name TEXT NOT NULL, date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP());");
}

bool MySQLDatabase::IsMigrationRun(const std::string_view str) {
	return ExecuteSelect("SELECT name FROM migration_history WHERE name = ?;", str)->next();
}

void MySQLDatabase::InsertMigration(const std::string_view str) {
	auto stmt = ExecuteInsert("INSERT INTO migration_history (name) VALUES (?);", str);
}

// charxml table

std::string MySQLDatabase::GetCharacterXml(const uint32_t charId) {
	auto result = ExecuteSelect("SELECT xml_data FROM charxml WHERE id = ? LIMIT 1;", charId);

	if (!result->next()) {
		return "";
	}

	return result->getString("xml_data").c_str();
}

void MySQLDatabase::UpdateCharacterXml(const uint32_t charId, const std::string_view lxfml) {
	ExecuteUpdate("UPDATE charxml SET xml_data = ? WHERE id = ?;", lxfml, charId);
}

void MySQLDatabase::InsertCharacterXml(const uint32_t accountId, const std::string_view lxfml) {
	ExecuteInsert("INSERT INTO `charxml` (`id`, `xml_data`) VALUES (?,?)", accountId, lxfml);
}

void MySQLDatabase::DeleteCharacter(const uint32_t characterId) {
	ExecuteDelete("DELETE FROM charxml WHERE id=? LIMIT 1;", characterId);
	ExecuteDelete("DELETE FROM command_log WHERE character_id=?;", characterId);
	ExecuteDelete("DELETE FROM friends WHERE player_id=? OR friend_id=?;", characterId, characterId);
	ExecuteDelete("DELETE FROM leaderboard WHERE character_id=?;", characterId);
	ExecuteDelete("DELETE FROM properties_contents WHERE property_id IN (SELECT id FROM properties WHERE owner_id=?);", characterId);
	ExecuteDelete("DELETE FROM properties WHERE owner_id=?;", characterId);
	ExecuteDelete("DELETE FROM ugc WHERE character_id=?;", characterId);
	ExecuteDelete("DELETE FROM activity_log WHERE character_id=?;", characterId);
	ExecuteDelete("DELETE FROM mail WHERE receiver_id=?;", characterId);
	ExecuteDelete("DELETE FROM charinfo WHERE id=? LIMIT 1;", characterId);
}

// pet_names table

void MySQLDatabase::SetPetNameModerationStatus(const LWOOBJID& petId, const std::string_view name, const int32_t approvalStatus) {
	auto stmt = ExecuteInsert(
		"INSERT INTO `pet_names` (`id`, `pet_name`, `approved`) VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE pet_name = ?, approved = ?;",
		petId,
		name,
		approvalStatus,
		name,
		approvalStatus);
}

std::optional<PetNameInfo> MySQLDatabase::GetPetNameInfo(const LWOOBJID& petId) {
	auto result = ExecuteSelect("SELECT pet_name, approved FROM pet_names WHERE id = ? LIMIT 1;", petId);

	if (!result->next()) {
		return std::nullopt;
	}

	PetNameInfo toReturn;
	toReturn.petName = result->getString("pet_name").c_str();
	toReturn.approvalStatus = result->getInt("approved");

	return toReturn;
}

// properties table

std::optional<PropertyInfo> MySQLDatabase::GetPropertyInfo(const uint32_t templateId, const LWOCLONEID cloneId) {
	auto propertyEntry = ExecuteSelect(
		"SELECT id, owner_id, clone_id, name, description, privacy_option, rejection_reason, last_updated, time_claimed, reputation, mod_approved "
		"FROM properties WHERE template_id = ? AND clone_id = ?;", templateId, cloneId);

	if (!propertyEntry->next()) {
		return std::nullopt;
	}

	PropertyInfo toReturn;
	toReturn.id = propertyEntry->getUInt64("id");
	toReturn.ownerId = propertyEntry->getUInt64("owner_id");
	toReturn.cloneId = propertyEntry->getUInt64("clone_id");
	toReturn.name = propertyEntry->getString("name").c_str();
	toReturn.description = propertyEntry->getString("description").c_str();
	toReturn.privacyOption = propertyEntry->getInt("privacy_option");
	toReturn.rejectionReason = propertyEntry->getString("rejection_reason").c_str();
	toReturn.lastUpdatedTime = propertyEntry->getUInt("last_updated");
	toReturn.claimedTime = propertyEntry->getUInt("time_claimed");
	toReturn.reputation = propertyEntry->getUInt("reputation");
	toReturn.modApproved = propertyEntry->getUInt("mod_approved");

	return toReturn;
}

void MySQLDatabase::UpdatePropertyModerationInfo(const LWOOBJID& id, const uint32_t privacyOption, const std::string_view rejectionReason, const uint32_t modApproved) {
	ExecuteUpdate("UPDATE properties SET privacy_option = ?, rejection_reason = ?, mod_approved = ? WHERE id = ? LIMIT 1;",
		privacyOption,
		rejectionReason,
		modApproved,
		id);
}

void MySQLDatabase::UpdatePropertyDetails(const LWOOBJID& id, const std::string_view name, const std::string_view description) {
	ExecuteUpdate("UPDATE properties SET name = ?, description = ? WHERE id = ? LIMIT 1;", name, description, id);
}

std::optional<PropertyModerationInfo> MySQLDatabase::GetPropertyModerationInfo(const LWOOBJID& propertyId) {
	auto result = ExecuteSelect("SELECT rejection_reason, mod_approved FROM properties WHERE id = ? LIMIT 1;", propertyId);
	if (!result->next()) {
		return std::nullopt;
	}

	PropertyModerationInfo toReturn;
	toReturn.rejectionReason = result->getString("rejection_reason").c_str();
	toReturn.modApproved = result->getUInt("mod_approved");

	return toReturn;
}

void MySQLDatabase::UpdatePerformanceCost(const LWOZONEID& zoneId, const float performanceCost) {
	ExecuteUpdate("UPDATE properties SET performance_cost = ? WHERE zone_id = ? AND clone_id = ? LIMIT 1;", performanceCost, zoneId.GetMapID(), zoneId.GetCloneID());
}

void MySQLDatabase::InsertNewProperty(
	const LWOOBJID& propertyId,
	const uint32_t characterId,
	const uint32_t templateId,
	const uint32_t cloneId,
	const std::string_view name,
	const std::string_view description,
	const uint32_t zoneId) {
	auto insertion = ExecuteInsert(
		"INSERT INTO properties"
		"(id, owner_id, template_id, clone_id, name, description, zone_id, rent_amount, rent_due, privacy_option, last_updated, time_claimed, rejection_reason, reputation, performance_cost)"
		"VALUES (?, ?, ?, ?, ?, ?, ?, 0, 0, 0, UNIX_TIMESTAMP(), UNIX_TIMESTAMP(), '', 0, 0.0)",
		propertyId,
		characterId,
		templateId,
		cloneId,
		name,
		description,
		zoneId
	);
}

// properties_contents table

std::vector<DatabaseModel> MySQLDatabase::GetPropertyModels(const LWOOBJID& propertyId) {
	auto result = ExecuteSelect("SELECT id, lot, x, y, z, rx, ry, rz, rw, ugc_id FROM properties_contents WHERE property_id = ?;", propertyId);

	std::vector<DatabaseModel> toReturn;
	toReturn.reserve(result->rowsCount());
	while (result->next()) {
		DatabaseModel model;
		model.id = result->getUInt64("id");
		model.lot = static_cast<LOT>(result->getUInt("lot"));
		model.position.x = result->getFloat("x");
		model.position.y = result->getFloat("y");
		model.position.z = result->getFloat("z");
		model.rotation.w = result->getFloat("rw");
		model.rotation.x = result->getFloat("rx");
		model.rotation.y = result->getFloat("ry");
		model.rotation.z = result->getFloat("rz");
		model.ugcId = result->getUInt64("ugc_id");
		toReturn.push_back(std::move(model));
	}
	return toReturn; // RVO; allow compiler to elide the return.
}

void MySQLDatabase::InsertNewPropertyModel(const LWOOBJID& propertyId, const DatabaseModel& model, const std::string_view name) {
	try {
		ExecuteInsert(
			"INSERT INTO properties_contents"
			"(id, property_id, ugc_id, lot, x, y, z, rx, ry, rz, rw, model_name, model_description, behavior_1, behavior_2, behavior_3, behavior_4, behavior_5)"
			"VALUES (?,  ?,           ?,      ?,   ?, ?, ?, ?,  ?,  ?,  ?,  ?,    ?,           ?,          ?,          ?,          ?,          ?)",
			//       1,  2,           3,      4,   5, 6, 7, 8,  9,  10, 11, 12,   13,          14,         15,         16,         17          18
			model.id, propertyId, model.ugcId == 0 ? std::nullopt : std::optional(model.ugcId), static_cast<uint32_t>(model.lot),
			model.position.x, model.position.y, model.position.z, model.rotation.x, model.rotation.y, model.rotation.z, model.rotation.w,
			name, "", // Model description.  TODO implement this.
			0, // behavior 1.  TODO implement this.
			0, // behavior 2.  TODO implement this.
			0, // behavior 3.  TODO implement this.
			0, // behavior 4.  TODO implement this.
			0 // behavior 5.  TODO implement this.
		);
	} catch (sql::SQLException& e) {
		LOG("Error inserting new property model: %s", e.what());
	}
}

void MySQLDatabase::UpdateModelPositionRotation(const LWOOBJID& propertyId, const NiPoint3& position, const NiQuaternion& rotation) {
	ExecuteUpdate(
		"UPDATE properties_contents SET x = ?, y = ?, z = ?, rx = ?, ry = ?, rz = ?, rw = ? WHERE id = ?;",
		position.x, position.y, position.z, rotation.x, rotation.y, rotation.z, rotation.w, propertyId);
}

void MySQLDatabase::RemoveModel(const LWOOBJID& modelId) {
	ExecuteDelete("DELETE FROM properties_contents WHERE id = ?;", modelId);
}

std::vector<LWOOBJID> MySQLDatabase::GetPropertyModelIds(const LWOOBJID& propertyId) {
	auto result = ExecuteSelect("SELECT id FROM properties_contents WHERE property_id = ?;", propertyId);

	std::vector<LWOOBJID> toReturn;
	toReturn.reserve(result->rowsCount());
	while (result->next()) {
		toReturn.push_back(result->getUInt64("id"));
	}
	return toReturn; // RVO; allow compiler to elide the return.
}

// bug_reports table

void MySQLDatabase::InsertNewBugReport(
	const std::string_view body,
	const std::string_view clientVersion,
	const std::string_view otherPlayer,
	const std::string_view selection,
	const uint32_t characterId) {
	ExecuteInsert("INSERT INTO `bug_reports`(body, client_version, other_player_id, selection, reporter_id) VALUES (?, ?, ?, ?, ?)",
		body, clientVersion, otherPlayer, selection, characterId);
}

// player_cheat_detections table

void MySQLDatabase::InsertCheatDetection(
	std::optional<uint32_t> userId,
	const std::string_view username,
	const std::string_view systemAddress,
	const std::string_view extraMessage) {
	ExecuteInsert(
		"INSERT INTO player_cheat_detections (account_id, name, violation_msg, violation_system_address) VALUES (?, ?, ?, ?)",
		userId, username, extraMessage, systemAddress);
}

// mail table

void MySQLDatabase::InsertNewMail(const MailInfo& mail) {
	ExecuteInsert(
		"INSERT INTO `mail` "
		"(`sender_id`, `sender_name`, `receiver_id`, `receiver_name`, `time_sent`, `subject`, `body`, `attachment_id`, `attachment_lot`, `attachment_subkey`, `attachment_count`, `was_read`)"
		" VALUES (?,?,?,?,?,?,?,?,?,?,?,0)",
		mail.senderId,
		mail.senderUsername.c_str(),
		mail.receiverId,
		mail.recipient.c_str(),
		time(NULL),
		mail.subject,
		mail.body,
		mail.itemID,
		mail.itemLOT,
		0,
		mail.itemCount);
}

std::vector<MailInfo> MySQLDatabase::GetMailForPlayer(const uint32_t numberOfMail, const uint32_t characterId) {
	auto res = ExecuteSelect(
		"SELECT id, subject, body, sender_name, attachment_id, attachment_lot, attachment_subkey, attachment_count, was_read, time_sent"
		" FROM mail WHERE receiver_id=? limit 20;",
		characterId);

	std::vector<MailInfo> toReturn;
	toReturn.reserve(res->rowsCount());

	while (res->next()) {
		MailInfo mail;
		mail.id = res->getUInt64("id");
		mail.subject = res->getString("subject").c_str();
		mail.body = res->getString("body").c_str();
		mail.senderUsername = res->getString("sender_name").c_str();
		mail.itemID = res->getUInt("attachment_id");
		mail.itemLOT = res->getInt("attachment_lot");
		mail.itemSubkey = res->getInt("attachment_subkey");
		mail.itemCount = res->getInt("attachment_count");
		mail.timeSent = res->getUInt64("time_sent");
		mail.wasRead = res->getBoolean("was_read");

		toReturn.push_back(std::move(mail));
	}

	return toReturn;
}

std::optional<MailInfo> MySQLDatabase::GetMail(const uint64_t mailId) {
	auto res = ExecuteSelect("SELECT attachment_lot, attachment_count FROM mail WHERE id=? LIMIT 1;", mailId);

	if (!res->next()) {
		return std::nullopt;
	}

	MailInfo toReturn;
	toReturn.itemLOT = res->getInt("attachment_lot");
	toReturn.itemCount = res->getInt("attachment_count");

	return toReturn;
}

uint32_t MySQLDatabase::GetUnreadMailCount(const uint32_t characterId) {
	auto res = ExecuteSelect("SELECT COUNT(*) as number_unread FROM mail WHERE receiver_id=? AND was_read=0;", characterId);

	if (!res->next()) {
		return 0;
	}

	return res->getInt("number_unread");
}

void MySQLDatabase::MarkMailRead(const uint64_t mailId) {
	ExecuteUpdate("UPDATE mail SET was_read=1 WHERE id=? LIMIT 1;", mailId);
}

void MySQLDatabase::DeleteMail(const uint64_t mailId) {
	ExecuteDelete("DELETE FROM mail WHERE id=? LIMIT 1;", mailId);
}

void MySQLDatabase::ClaimMailItem(const uint64_t mailId) {
	ExecuteUpdate("UPDATE mail SET attachment_lot=0 WHERE id=? LIMIT 1;", mailId);
}

// command_log table

void MySQLDatabase::InsertSlashCommandUsage(const std::string_view command, const uint32_t characterId) {
	ExecuteInsert("INSERT INTO command_log (character_id, command) VALUES (?, ?);", characterId, command);
}

// servers table

void MySQLDatabase::SetMasterIp(const std::string_view ip, const uint32_t port) {
	// We only want our 1 entry anyways, so we can just delete all and reinsert the one we want
	// since it would be two queries anyways.
	ExecuteDelete("TRUNCATE TABLE servers;");
	ExecuteInsert("INSERT INTO `servers` (`name`, `ip`, `port`, `state`, `version`) VALUES ('master', ?, ?, 0, 171022)", ip, port);
}

std::optional<MasterInfo> MySQLDatabase::GetMasterInfo() {
	auto result = ExecuteSelect("SELECT ip, port FROM servers WHERE name='master' LIMIT 1;");

	if (!result->next()) {
		return std::nullopt;
	}

	MasterInfo toReturn;

	toReturn.ip = result->getString("ip").c_str();
	toReturn.port = result->getInt("port");

	return toReturn;
}

// object_id_tracker table

std::optional<uint32_t> MySQLDatabase::GetCurrentPersistentId() {
	auto result = ExecuteSelect("SELECT last_object_id FROM object_id_tracker");
	if (!result->next()) {
		return std::nullopt;
	}
	return result->getUInt("last_object_id");
}

void MySQLDatabase::InsertDefaultPersistentId() {
	ExecuteInsert("INSERT INTO object_id_tracker VALUES (1);");
}

void MySQLDatabase::UpdatePersistentId(const uint32_t newId) {
	ExecuteUpdate("UPDATE object_id_tracker SET last_object_id = ?;", newId);
}

// leaderboard table

std::optional<uint32_t> MySQLDatabase::GetDonationTotal(const uint32_t activityId) {
	auto donation_total = ExecuteSelect("SELECT SUM(primaryScore) as donation_total FROM leaderboard WHERE game_id = ?;", activityId);

	if (!donation_total->next()) {
		return std::nullopt;
	}

	return donation_total->getUInt("donation_total");
}

// play_keys table

std::optional<bool> MySQLDatabase::IsPlaykeyActive(const uint32_t playkeyId) {
	auto keyCheckRes = ExecuteSelect("SELECT active FROM `play_keys` WHERE id=?", playkeyId);

	if (!keyCheckRes->next()) {
		return std::nullopt;
	}

	return keyCheckRes->getBoolean("active");
}
