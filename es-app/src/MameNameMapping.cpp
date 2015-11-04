#include "MameNameMapping.h"
#include "Log.h"

#include <string.h>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sqlite3/sqlite3.h>
#include "SqliteHelper.h"

using namespace std;

sqlite3* mameNameDb = NULL;

// Defined in main.cpp
extern std::string ExecutablePath;

namespace MameNameMapping
{
  void init() {
    std::string dbPath = ExecutablePath + string("/data/MameMapping.sqlite");
    LOG(LogDebug) << "Loading mame mapping db from " << dbPath;

    if(sqlite3_open_v2(dbPath.c_str(), &mameNameDb, SQLITE_OPEN_READONLY, NULL))
    {
      throw DBException() << "Could not open database \"" << dbPath << "\".\n"
        "\t" << sqlite3_errmsg(mameNameDb);
    }
  }

  std::string getLongName(const std::string& platform, const std::string& shortName)
  {
    SQLPreparedStmt readStmt(mameNameDb, "SELECT longname FROM t WHERE platform = ?1 AND shortname = ?2");
    sqlite3_bind_text(readStmt, 1, platform.c_str(), platform.size(), SQLITE_STATIC);
    sqlite3_bind_text(readStmt, 2, shortName.c_str(), shortName.size(), SQLITE_STATIC);

    int result = readStmt.step();

    if (result == SQLITE_DONE) {
      return std::string("");
    }

    if (result != SQLITE_ROW) {
      throw DBException() << "Step failed (got " << result << ", expected " << SQLITE_ROW << "!\n\t" << sqlite3_errmsg(mameNameDb);
    }

    const unsigned char *longname = sqlite3_column_text(readStmt, 0);
    return std::string((const char *)longname);
  }

  std::string shortToFullName(const std::vector<std::string>& platformIds, const std::string &shortName)
	{
    if (!mameNameDb) {
      MameNameMapping::init();
    }
	  for (std::string platformId : platformIds) {
      std::string longName = getLongName(platformId, shortName);
      if (longName.length()) {
        return longName;
      }
	  }
    std::string allIds;
	  for (std::string platformId : platformIds) {
      if (allIds.length()) {
        allIds += string("+");
      }
      allIds += platformId;
    }
	  LOG(LogDebug) << allIds << "/" << shortName << " has no mapping.";
	  return shortName;
	}
}
