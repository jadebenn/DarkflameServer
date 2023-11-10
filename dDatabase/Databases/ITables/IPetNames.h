#ifndef __IPETNAMES__H__
#define __IPETNAMES__H__

#include <cstdint>
#include <optional>

class IPetNames {
public:
	struct Info {
		std::string petName;
		int32_t approvalStatus{};
	};

	virtual void SetPetNameModerationStatus(const LWOOBJID& petId, const std::string_view name, const int32_t approvalStatus) = 0;
	virtual std::optional<IPetNames::Info> GetPetNameInfo(const LWOOBJID& petId) = 0;
};

#endif  //!__IPETNAMES__H__
