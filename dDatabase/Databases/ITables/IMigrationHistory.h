#ifndef __IMIGRATIONHISTORY__H__
#define __IMIGRATIONHISTORY__H__

#include <string_view>

class IMigrationHistory {
public:
	virtual void CreateMigrationHistoryTable() = 0;
	virtual bool IsMigrationRun(const std::string_view str) = 0;
	virtual void InsertMigration(const std::string_view str) = 0;
};
#endif  //!__IMIGRATIONHISTORY__H__
