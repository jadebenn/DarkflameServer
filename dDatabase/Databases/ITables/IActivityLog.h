#ifndef __IACTIVITYLOG__H__
#define __IACTIVITYLOG__H__

#include <cstdint>

#include "dCommonVars.h"

enum class eActivityType : uint32_t {
	PlayerLoggedOut,
	PlayerLoggedIn,
};

class IActivityLog {
public:
	virtual void UpdateActivityLog(const uint32_t accountId, const eActivityType activityType, const LWOMAPID mapId) = 0;
};

#endif  //!__IACTIVITYLOG__H__
