#ifndef __IOBJECTIDTRACKER__H__
#define __IOBJECTIDTRACKER__H__

#include <cstdint>
#include <optional>

class IObjectIdTracker {
public:
	virtual std::optional<uint32_t> GetCurrentPersistentId() = 0;
	virtual void InsertDefaultPersistentId() = 0;
	virtual void UpdatePersistentId(const uint32_t newId) = 0;
};

#endif  //!__IOBJECTIDTRACKER__H__
