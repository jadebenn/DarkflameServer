#ifndef __ICOMMANDLOG__H__
#define __ICOMMANDLOG__H__

#include <cstdint>
#include <string_view>

class ICommandLog {
public:	
	virtual void InsertSlashCommandUsage(const std::string_view command, const uint32_t characterId) = 0;
};

#endif  //!__ICOMMANDLOG__H__
