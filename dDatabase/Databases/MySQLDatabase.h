#ifndef __MYSQLDATABASE__H__
#define __MYSQLDATABASE__H__

#include <conncpp.hpp>
#include <memory>

#include "GameDatabase.h"

typedef std::unique_ptr<sql::PreparedStatement>& UniquePreppedStmtRef;

// Purposefully no definition for this to provide linker errors in the case someone tries to
// bind a parameter to a type that isn't defined.
template<typename ParamType>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const ParamType param);

template<typename... Args>
void SetParams(UniquePreppedStmtRef stmt, Args&&... args) {
	if constexpr (sizeof...(args) != 0) {
		int i = 1;
		(SetParam(stmt, i++, args), ...);
	}
}

class MySQLDatabase : public GameDatabase {
public:
	void Connect() override;
	void Destroy(std::string source = "", bool log = true) override;

	sql::PreparedStatement* CreatePreppedStmt(const std::string& query) override;
	void Commit() override;
	bool GetAutoCommit() override;
	void SetAutoCommit(bool value) override;
	void ExecuteCustomQuery(const std::string_view query) override;

	// Overloaded queries
	std::optional<DatabaseStructs::MasterInfo> GetMasterInfo() override;

	std::optional<DatabaseStructs::ApprovedNames> GetApprovedCharacterNames() override;

	std::optional<DatabaseStructs::FriendsList> GetFriendsList(uint32_t charID) override;

	// No optional needed here, since if we did that, we'd return a bool of a bool in essenece.
	// Just return true if and only if the character name exists.
	std::optional<uint32_t> DoesCharacterExist(const std::string_view name) override;
	std::optional<DatabaseStructs::BestFriendStatus> GetBestFriendStatus(const uint32_t playerAccountId, const uint32_t friendAccountId) override;
	void SetBestFriendStatus(const uint32_t playerAccountId, const uint32_t friendAccountId, const uint32_t bestFriendStatus) override;
	void AddFriend(const uint32_t playerAccountId, const uint32_t friendAccountId) override;
	std::optional<uint32_t> GetCharacterIdFromCharacterName(const std::string_view name) override;
	void RemoveFriend(const uint32_t playerAccountId, const uint32_t friendAccountId) override;
	void UpdateActivityLog(const uint32_t accountId, const eActivityType activityType, const LWOMAPID mapId) override;
	void DeleteUgcModelData(const LWOOBJID& modelId) override;
	void UpdateUgcModelData(const LWOOBJID& modelId, std::istringstream& lxfml) override;
	std::vector<DatabaseStructs::UgcModel> GetUgcModels() override;
	void CreateMigrationHistoryTable() override;
	bool IsMigrationRun(const std::string_view str) override;
	void InsertMigration(const std::string_view str) override;
	std::optional<DatabaseStructs::CharacterInfo> GetCharacterInfo(const uint32_t charId) override;
	std::optional<DatabaseStructs::CharacterInfo> GetCharacterInfo(const std::string_view charId) override;
	std::string GetCharacterXml(const uint32_t accountId) override;
	void UpdateCharacterXml(const uint32_t accountId, const std::string_view lxfml) override;
	std::optional<DatabaseStructs::AccountInfo> GetAccountInfo(const std::string_view username) override;
	void InsertNewCharacter(const uint32_t accountId, const uint32_t characterId, const std::string_view name, const std::string_view pendingName) override;
	void InsertCharacterXml(const uint32_t accountId, const std::string_view lxfml) override;
	std::vector<uint32_t> GetCharacterIds(uint32_t accountId) override;
	void DeleteCharacter(const uint32_t characterId) override;
	void SetCharacterName(const uint32_t characterId, const std::string_view name) override;
	void SetPendingCharacterName(const uint32_t characterId, const std::string_view name) override;
	void UpdateLastLoggedInCharacter(const uint32_t characterId) override;
	void SetPetNameModerationStatus(const LWOOBJID& petId, const std::string_view name, const int32_t approvalStatus) override;
	std::optional<DatabaseStructs::PetNameInfo> GetPetNameInfo(const LWOOBJID& petId) override;
	std::optional<DatabaseStructs::PropertyInfo> GetPropertyInfo(const uint32_t templateId, const LWOCLONEID cloneId) override;
	void UpdatePropertyModerationInfo(const LWOOBJID& id, const uint32_t privacyOption, const std::string_view rejectionReason, const uint32_t modApproved) override;
	void UpdatePropertyDetails(const LWOOBJID& id, const std::string_view name, const std::string_view description) override;
	void InsertNewProperty(
		const LWOOBJID& propertyId,
		const uint32_t characterId,
		const uint32_t templateId,
		const uint32_t cloneId,
		const std::string_view name,
		const std::string_view description,
		const uint32_t zoneId) override;
	std::vector<DatabaseStructs::DatabaseModel> GetPropertyModels(const LWOOBJID& propertyId) override;
	void RemoveUnreferencedUgcModels() override;
	void InsertNewPropertyModel(const LWOOBJID& propertyId, const DatabaseStructs::DatabaseModel& model, const std::string_view name) override;
	void UpdateModelPositionRotation(const LWOOBJID& propertyId, const NiPoint3& position, const NiQuaternion& rotation) override;
	void RemoveModel(const LWOOBJID& modelId) override;
	std::vector<LWOOBJID> GetPropertyModelIds(const LWOOBJID& propertyId) override;
	std::optional<DatabaseStructs::PropertyModerationInfo> GetPropertyModerationInfo(const LWOOBJID& propertyId) override;
	void UpdatePerformanceCost(const LWOZONEID& zoneId, const float performanceCost) override;
	void InsertNewBugReport(
		const std::string_view body,
		const std::string_view clientVersion,
		const std::string_view otherPlayer,
		const std::string_view selection,
		const uint32_t characterId) override;
	void InsertCheatDetection(
		std::optional<uint32_t> userId,
		const std::string_view username,
		const std::string_view systemAddress,
		const std::string_view extraMessage) override;
	void InsertNewMail(const DatabaseStructs::MailInfo& mail) override;
	void InsertNewUgcModel(
		std::istringstream& sd0Data,
		const uint32_t blueprintId,
		const uint32_t accountId,
		const uint32_t characterId) override;
	std::vector<DatabaseStructs::MailInfo> GetMailForPlayer(const uint32_t numberOfMail, const uint32_t characterId) override;
	std::optional<DatabaseStructs::MailInfo> GetMail(const uint64_t mailId) override;
	uint32_t GetUnreadMailCount(const uint32_t characterId) override;
	void MarkMailRead(const uint64_t mailId) override;
	void DeleteMail(const uint64_t mailId) override;
	void ClaimMailItem(const uint64_t mailId) override;
	void InsertSlashCommandUsage(const std::string_view command, const uint32_t characterId) override;
	void UpdateAccountUnmuteTime(const uint32_t accountId, const uint64_t timeToUnmute) override;
	void UpdateAccountBan(const uint32_t accountId, const bool banned) override;
	void UpdateAccountPassword(const std::string_view bcryptpassword, const uint32_t accountId) override;
	void InsertNewAccount(const std::string_view username, const std::string_view bcryptpassword) override;
	void SetMasterIp(const std::string_view ip, const uint32_t port) override;
	std::optional<uint32_t> GetCurrentPersistentId() override;
	void InsertDefaultPersistentId() override;
	void UpdatePersistentId(const uint32_t id) override;
	std::optional<uint32_t> GetDonationTotal(const uint32_t activityId) override;
	std::optional<bool> IsPlaykeyActive(const uint32_t playkeyId) override;
	std::vector<DatabaseStructs::UgcModel> GetAllUgcModels(const LWOOBJID& propertyId) override;
private:
	template<typename... Args>
	inline std::unique_ptr<sql::ResultSet> ExecuteSelect(const std::string& query, Args&&... args) {
		std::unique_ptr<sql::PreparedStatement> preppedStmt(CreatePreppedStmt(query));
		SetParams(preppedStmt, std::forward<Args>(args)...);
		return std::unique_ptr<sql::ResultSet>(preppedStmt->executeQuery());
	}

	template<typename... Args>
	inline void ExecuteDelete(const std::string& query, Args&&... args) {
		std::unique_ptr<sql::PreparedStatement> preppedStmt(CreatePreppedStmt(query));
		SetParams(preppedStmt, std::forward<Args>(args)...);
		preppedStmt->execute();
	}

	template<typename... Args>
	inline int32_t ExecuteUpdate(const std::string& query, Args&&... args) {
		std::unique_ptr<sql::PreparedStatement> preppedStmt(CreatePreppedStmt(query));
		SetParams(preppedStmt, std::forward<Args>(args)...);
		return preppedStmt->executeUpdate();
	}

	template<typename... Args>
	inline bool ExecuteInsert(const std::string& query, Args&&... args) {
		std::unique_ptr<sql::PreparedStatement> preppedStmt(CreatePreppedStmt(query));
		SetParams(preppedStmt, std::forward<Args>(args)...);
		return preppedStmt->execute();
	}
};

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const std::string_view param) {
	// LOG("%s", param.data());
	stmt->setString(index, param.data());
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const char* param) {
	// LOG("%s", param);
	stmt->setString(index, param);
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const std::string param) {
	// LOG("%s", param.c_str());
	stmt->setString(index, param.c_str());
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const int8_t param) {
	// LOG("%u", param);
	stmt->setByte(index, param);
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const uint8_t param) {
	// LOG("%d", param);
	stmt->setByte(index, param);
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const int16_t param) {
	// LOG("%u", param);
	stmt->setShort(index, param);
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const uint16_t param) {
	// LOG("%d", param);
	stmt->setShort(index, param);
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const uint32_t param) {
	// LOG("%u", param);
	stmt->setUInt(index, param);
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const int32_t param) {
	// LOG("%d", param);
	stmt->setInt(index, param);
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const int64_t param) {
	// LOG("%llu", param);
	stmt->setInt64(index, param);
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const uint64_t param) {
	// LOG("%llu", param);
	stmt->setUInt64(index, param);
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const float param) {
	// LOG("%f", param);
	stmt->setFloat(index, param);
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const double param) {
	// LOG("%f", param);
	stmt->setDouble(index, param);
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const bool param) {
	// LOG("%d", param);
	stmt->setBoolean(index, param);
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const std::istream* param) {
	// LOG("Blob");
	// This is the one time you will ever see me use const_cast.
	stmt->setBlob(index, const_cast<std::istream*>(param));
}

template<>
inline void SetParam(UniquePreppedStmtRef stmt, const int index, const std::optional<uint32_t> param) {
	if (param) {
		// LOG("%d", param.value());
		stmt->setInt(index, param.value());
	} else {
		// LOG("Null");
		stmt->setNull(index, sql::DataType::SQLNULL);
	}
}

#endif  //!__MYSQLDATABASE__H__
