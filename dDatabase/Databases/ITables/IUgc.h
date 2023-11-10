#ifndef __IUGC__H__
#define __IUGC__H__

#include <cstdint>
#include <sstream>
#include <optional>
#include <string>
#include <string_view>

class IUgc {
public:
	struct Model {
		std::stringstream lxfmlData;
		LWOOBJID id{};
	};

	virtual std::vector<IUgc::Model> GetAllUgcModels(const LWOOBJID& propertyId) = 0;
	virtual std::vector<IUgc::Model> GetUgcModels() = 0;
	virtual void RemoveUnreferencedUgcModels() = 0;
	virtual void InsertNewProperty(
		const LWOOBJID& propertyId,
		const uint32_t characterId,
		const uint32_t templateId,
		const uint32_t cloneId,
		const std::string_view name,
		const std::string_view description,
		const uint32_t zoneId) = 0;
	virtual void DeleteUgcModelData(const LWOOBJID& modelId) = 0;
	virtual void UpdateUgcModelData(const LWOOBJID& modelId, std::istringstream& lxfml) = 0;
};
#endif  //!__IUGC__H__
