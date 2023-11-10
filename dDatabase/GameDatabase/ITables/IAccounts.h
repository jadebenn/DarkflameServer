#ifndef __IACCOUNTS__H__
#define __IACCOUNTS__H__

#include <cstdint>
#include <optional>
#include <string_view>

enum class eGameMasterLevel : uint8_t;

class IAccounts {
public:
	struct Info {
		std::string bcryptPassword;
		uint32_t id{};
		uint32_t playKeyId{};
		bool banned{};
		bool locked{};
		eGameMasterLevel maxGmLevel{};
	};

	virtual std::optional<IAccounts::Info> GetAccountInfo(const std::string_view username) = 0;
	virtual void UpdateAccountUnmuteTime(const uint32_t accountId, const uint64_t timeToUnmute) = 0;
	virtual void UpdateAccountBan(const uint32_t accountId, const bool banned) = 0;
	virtual void UpdateAccountPassword(const std::string_view bcryptpassword, const uint32_t accountId) = 0;
	virtual void InsertNewAccount(const std::string_view username, const std::string_view bcryptpassword) = 0;
};

#endif  //!__IACCOUNTS__H__
