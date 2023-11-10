#ifndef __IPROPERTIESCONTENTS__H__
#define __IPROPERTIESCONTENTS__H__

#include <cstdint>
#include <string_view>

class IPropertyContents {
public:	
	struct Model {
		bool operator==(const LWOOBJID& other) const {
			return id == other;
		}

		NiPoint3 position;
		NiQuaternion rotation;
		LWOOBJID id{};
		LOT lot{};
		uint32_t ugcId{};
	};

	virtual void InsertNewUgcModel(
		std::istringstream& sd0Data,
		const uint32_t blueprintId,
		const uint32_t accountId,
		const uint32_t characterId) = 0;
	virtual std::vector<IPropertyContents::Model> GetPropertyModels(const LWOOBJID& propertyId) = 0;
	virtual void InsertNewPropertyModel(const LWOOBJID& propertyId, const IPropertyContents::Model& model, const std::string_view name) = 0;
	virtual void UpdateModelPositionRotation(const LWOOBJID& propertyId, const NiPoint3& position, const NiQuaternion& rotation) = 0;
	virtual void RemoveModel(const LWOOBJID& modelId) = 0;
};
#endif  //!__IPROPERTIESCONTENTS__H__
