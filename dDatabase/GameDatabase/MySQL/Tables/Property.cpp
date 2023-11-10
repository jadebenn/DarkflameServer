#include "MySQLDatabase.h"

std::optional<IProperty::PropertyInfo> MySQLDatabase::GetPropertyInfo(const LWOMAPID mapId, const LWOCLONEID cloneId) {
	auto propertyEntry = ExecuteSelect(
		"SELECT id, owner_id, clone_id, name, description, privacy_option, rejection_reason, last_updated, time_claimed, reputation, mod_approved "
		"FROM properties WHERE zone_id = ? AND clone_id = ?;", mapId, cloneId);

	if (!propertyEntry->next()) {
		return std::nullopt;
	}

	IProperty::PropertyInfo toReturn;
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
