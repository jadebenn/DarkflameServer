#ifndef __IMAIL__H__
#define __IMAIL__H__

#include <cstdint>
#include <optional>
#include <string_view>

#include "dCommonVars.h"
#include "NiQuaternion.h"
#include "NiPoint3.h"

class IMail {
public:
	struct MailInfo {
		std::string senderUsername;
		std::string recipient;
		std::string subject;
		std::string body;
		uint64_t id{};
		uint32_t senderId{};
		uint32_t receiverId{};
		uint64_t timeSent{};
		bool wasRead{};
		struct {
			LWOOBJID itemID{};
			int32_t itemCount{};
			LOT itemLOT{};
			LWOOBJID itemSubkey{};
		};
	};

	virtual void InsertNewMail(const MailInfo& mail) = 0;
	virtual std::vector<MailInfo> GetMailForPlayer(const uint32_t numberOfMail, const uint32_t characterId) = 0;
	virtual std::optional<MailInfo> GetMail(const uint64_t mailId) = 0;
	virtual uint32_t GetUnreadMailCount(const uint32_t characterId) = 0;
	virtual void MarkMailRead(const uint64_t mailId) = 0;
	virtual void ClaimMailItem(const uint64_t mailId) = 0;
	virtual void DeleteMail(const uint64_t mailId) = 0;
};

#endif  //!__IMAIL__H__
