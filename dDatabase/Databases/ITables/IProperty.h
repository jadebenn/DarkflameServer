#ifndef __IPROPERTY__H__
#define __IPROPERTY__H__

#include <cstdint>
#include <optional>

class IProperty {
public:
	struct PropertyInfo {
		std::string name;
		std::string description;
		std::string rejectionReason;
		LWOOBJID id{};
		LWOOBJID ownerId{};
		LWOCLONEID cloneId{};
		int32_t privacyOption{};
		uint32_t modApproved{};
		uint32_t lastUpdatedTime{};
		uint32_t claimedTime{};
		uint32_t reputation{};
	};
	virtual std::optional<IProperty::PropertyInfo> GetPropertyInfo(const LWOMAPID mapId, const LWOCLONEID cloneId) = 0;
	virtual void UpdatePropertyModerationInfo(const LWOOBJID& id, const uint32_t privacyOption, const std::string_view rejectionReason, const uint32_t modApproved) = 0;
	virtual void UpdatePropertyDetails(const LWOOBJID& id, const std::string_view name, const std::string_view description) = 0;
	virtual void UpdatePerformanceCost(const LWOZONEID& zoneId, const float performanceCost) = 0;
};
#endif  //!__IPROPERTY__H__
