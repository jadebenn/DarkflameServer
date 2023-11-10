#ifndef __IBUGREPORTS__H__
#define __IBUGREPORTS__H__

#include <cstdint>
#include <string_view>

class IBugReports {
public:
	virtual void InsertNewBugReport(const std::string_view body, const std::string_view clientVersion, const std::string_view otherPlayer, const std::string_view selection, const uint32_t characterId) = 0;
};
#endif  //!__IBUGREPORTS__H__
