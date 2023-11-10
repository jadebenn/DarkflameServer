#ifndef __IPLAYKEYS__H__
#define __IPLAYKEYS__H__

#include <cstdint>
#include <optional>

class IPlayKeys {
public:
	virtual std::optional<bool> IsPlaykeyActive(const uint32_t playkeyId) = 0;
};

#endif  //!__IPLAYKEYS__H__
