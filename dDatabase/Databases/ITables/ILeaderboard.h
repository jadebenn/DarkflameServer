#ifndef __ILEADERBOARD__H__
#define __ILEADERBOARD__H__

#include <cstdint>
#include <optional>

class ILeaderboard {
public:
	virtual std::optional<uint32_t> GetDonationTotal(const uint32_t activityId) = 0;
};

#endif  //!__ILEADERBOARD__H__
