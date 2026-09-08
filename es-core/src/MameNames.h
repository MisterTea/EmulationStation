//  SPDX-License-Identifier: MIT
//
//  ES-DE Frontend
//  MameNames.h
//
//  Provides expanded game names based on short MAME name arguments. Also contains
//  functions to check whether a passed argument is a MAME BIOS or a MAME device.
//  The data sources are stored as MameMapping.sqlite and the legacy XML files.
//

#ifndef ES_CORE_MAMENAMES_H
#define ES_CORE_MAMENAMES_H

#include "Settings.h"
#include "utils/StringUtil.h"

#include <algorithm>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

struct sqlite3;
struct sqlite3_stmt;

// Expand MAME names to full game names and lookup device and BIOS entries.
class MameNames
{
public:
    static MameNames& getInstance();
    ~MameNames();

    std::string getRealName(const std::string& platform, const std::string& mameName);
    std::string getCleanName(const std::string& platform, const std::string& mameName);
    std::vector<std::pair<std::string, std::string>> getGamesForPlatform(const std::string& platform);

    std::string getRealName(const std::string& mameName)
    {
        return getRealName("arcade", mameName);
    }

    std::string getCleanName(const std::string& mameName)
    {
        return getCleanName("arcade", mameName);
    }

    const bool isBios(const std::string& biosName)
    {
        return std::find(mMameBioses.cbegin(), mMameBioses.cend(), biosName) != mMameBioses.cend();
    }

    const bool isDevice(const std::string& deviceName)
    {
        return std::find(mMameDevices.cbegin(), mMameDevices.cend(), deviceName) !=
               mMameDevices.cend();
    }

private:
    MameNames();
    std::string queryDatabase(const std::string& platform, const std::string& mameName);
    std::vector<std::string> getPlatformCandidates(const std::string& platform);

    sqlite3* mDb;
    sqlite3_stmt* mPlatformStmt;
    sqlite3_stmt* mShortNameStmt;
    std::mutex mDbMutex;

    std::unordered_map<std::string, std::string> mCache;
    std::unordered_map<std::string, std::string> mNamePairs;
    std::vector<std::string> mMameBioses;
    std::vector<std::string> mMameDevices;
};

#endif // ES_CORE_MAMENAMES_H
