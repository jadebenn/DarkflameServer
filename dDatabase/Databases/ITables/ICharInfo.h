#ifndef __ICHARINFO__H__
#define __ICHARINFO__H__

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "ePermissionMap.h"

class ICharInfo {
public:
	struct Info {
		std::string name;
		std::string pendingName;
		uint32_t id{};
		uint32_t accountId{};
		bool needsRename{};
		LWOCLONEID cloneId{};
		ePermissionMap permissionMap{};
	};

	virtual std::vector<std::string> GetApprovedCharacterNames() = 0;
	virtual std::optional<uint32_t> DoesCharacterExist(const std::string_view name) = 0;
	virtual std::optional<uint32_t> GetCharacterIdFromCharacterName(const std::string_view name) = 0;
	virtual std::optional<ICharInfo::Info> GetCharacterInfo(const uint32_t charId) = 0;
	virtual std::optional<ICharInfo::Info> GetCharacterInfo(const std::string_view name) = 0;
	virtual std::vector<uint32_t> GetCharacterIds(const uint32_t accountId) = 0;
	virtual void InsertNewCharacter(const uint32_t accountId, const uint32_t characterId, const std::string_view name, const std::string_view pendingName) = 0;
	virtual void SetCharacterName(const uint32_t characterId, const std::string_view name) = 0;
	virtual void SetPendingCharacterName(const uint32_t characterId, const std::string_view name) = 0;
	virtual void UpdateLastLoggedInCharacter(const uint32_t characterId) = 0;
};

#endif  //!__ICHARINFO__H__
