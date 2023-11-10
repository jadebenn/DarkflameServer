#ifndef __IPLAYERCHEATDETECTIONS__H__
#define __IPLAYERCHEATDETECTIONS__H__

#include <cstdint>
#include <optional>

class IPlayerCheatDetections {
public:
	virtual void InsertCheatDetection(
		std::optional<uint32_t> userId,
		const std::string_view username,
		const std::string_view systemAddress,
		const std::string_view extraMessage) = 0;
};

#endif  //!__IPLAYERCHEATDETECTIONS__H__
