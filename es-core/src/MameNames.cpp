//  SPDX-License-Identifier: MIT
//
//  ES-DE Frontend
//  MameNames.cpp
//
//  Provides expanded game names based on short MAME name arguments. Also contains
//  functions to check whether a passed argument is a MAME BIOS or a MAME device.
//  The data sources are stored as MameMapping.sqlite and the legacy XML files.
//

#include "MameNames.h"

#include "Log.h"
#include "resources/ResourceManager.h"
#include "utils/FileSystemUtil.h"

#include <pugixml.hpp>
#include <sqlite3.h>
#include <unordered_set>

MameNames& MameNames::getInstance()
{
    static MameNames instance;
    return instance;
}

MameNames::MameNames()
    : mDb {nullptr}
    , mPlatformStmt {nullptr}
    , mShortNameStmt {nullptr}
{
    // Initialize SQLite MameMapping database.
    std::string dbPath {ResourceManager::getInstance().getResourcePath(":/MAME/MameMapping.sqlite")};
    if (Utils::FileSystem::exists(dbPath)) {
        LOG(LogInfo) << "Opening MAME mapping database \"" << dbPath << "\"...";
        if (sqlite3_open_v2(dbPath.c_str(), &mDb, SQLITE_OPEN_READONLY, nullptr) == SQLITE_OK) {
            const char* platformQuery =
                "SELECT longname FROM t WHERE platform = ?1 AND shortname = ?2 LIMIT 1;";
            if (sqlite3_prepare_v2(mDb, platformQuery, -1, &mPlatformStmt, nullptr) != SQLITE_OK) {
                LOG(LogError) << "Failed to prepare MAME platform statement: "
                              << sqlite3_errmsg(mDb);
            }
            const char* shortNameQuery =
                "SELECT longname FROM t WHERE shortname = ?1 LIMIT 1;";
            if (sqlite3_prepare_v2(mDb, shortNameQuery, -1, &mShortNameStmt, nullptr) != SQLITE_OK) {
                LOG(LogError) << "Failed to prepare MAME shortname statement: "
                              << sqlite3_errmsg(mDb);
            }
        }
        else {
            LOG(LogError) << "Failed to open MAME mapping database \"" << dbPath << "\": "
                          << (mDb ? sqlite3_errmsg(mDb) : "unknown error");
            if (mDb) {
                sqlite3_close(mDb);
                mDb = nullptr;
            }
        }
    }
    else {
        LOG(LogWarning) << "MAME mapping database \"" << dbPath << "\" not found.";
    }

    // Read legacy XML MAME names if available as fallback.
    std::string xmlpath {ResourceManager::getInstance().getResourcePath(":/MAME/mamenames.xml")};
    if (Utils::FileSystem::exists(xmlpath)) {
        pugi::xml_document doc;
        pugi::xml_parse_result result {doc.load_file(xmlpath.c_str())};
        if (result) {
            const pugi::xml_node& root {doc.child("mamenames")};
            if (root != nullptr) {
                for (pugi::xml_node gameNode {root.child("game")}; gameNode;
                     gameNode = gameNode.next_sibling("game")) {
                    mNamePairs[gameNode.child("mamename").text().get()] =
                        gameNode.child("realname").text().get();
                }
            }
            else {
                for (pugi::xml_node gameNode {doc.child("game")}; gameNode;
                     gameNode = gameNode.next_sibling("game")) {
                    mNamePairs[gameNode.child("mamename").text().get()] =
                        gameNode.child("realname").text().get();
                }
            }
        }
    }

    // Read BIOS file.
    xmlpath = ResourceManager::getInstance().getResourcePath(":/MAME/mamebioses.xml");
    if (Utils::FileSystem::exists(xmlpath)) {
        pugi::xml_document doc;
        pugi::xml_parse_result result {doc.load_file(xmlpath.c_str())};
        if (result) {
            const pugi::xml_node& root {doc.child("mamebioses")};
            if (root != nullptr) {
                for (pugi::xml_node biosNode {root.child("bios")}; biosNode;
                     biosNode = biosNode.next_sibling("bios")) {
                    mMameBioses.emplace_back(biosNode.text().get());
                }
            }
            else {
                for (pugi::xml_node biosNode {doc.child("bios")}; biosNode;
                     biosNode = biosNode.next_sibling("bios")) {
                    mMameBioses.emplace_back(biosNode.text().get());
                }
            }
        }
    }

    // Read device file.
    xmlpath = ResourceManager::getInstance().getResourcePath(":/MAME/mamedevices.xml");
    if (Utils::FileSystem::exists(xmlpath)) {
        pugi::xml_document doc;
        pugi::xml_parse_result result {doc.load_file(xmlpath.c_str())};
        if (result) {
            const pugi::xml_node& root {doc.child("mamedevices")};
            if (root != nullptr) {
                for (pugi::xml_node deviceNode {root.child("device")}; deviceNode;
                     deviceNode = deviceNode.next_sibling("device")) {
                    mMameDevices.emplace_back(deviceNode.text().get());
                }
            }
            else {
                for (pugi::xml_node deviceNode {doc.child("device")}; deviceNode;
                     deviceNode = deviceNode.next_sibling("device")) {
                    mMameDevices.emplace_back(deviceNode.text().get());
                }
            }
        }
    }
}

MameNames::~MameNames()
{
    if (mPlatformStmt) {
        sqlite3_finalize(mPlatformStmt);
        mPlatformStmt = nullptr;
    }
    if (mShortNameStmt) {
        sqlite3_finalize(mShortNameStmt);
        mShortNameStmt = nullptr;
    }
    if (mDb) {
        sqlite3_close(mDb);
        mDb = nullptr;
    }
}

std::vector<std::string> MameNames::getPlatformCandidates(const std::string& platform)
{
    std::string p {Utils::String::toLower(platform)};
    std::vector<std::string> candidates;
    candidates.push_back(p);

    if (p == "snes" || p == "sfc") {
        // Only NTSC snes, remove snespal
    }
    else if (p == "nes" || p == "famicom") {
        // Only nes, remove nespal
        candidates.push_back("nes");
    }
    else if (p == "genesis" || p == "megadrive") {
        candidates.push_back("genesis");
        candidates.push_back("megadriv");
    }
    else if (p == "mastersystem" || p == "sms") {
        candidates.push_back("sms");
        candidates.push_back("sms1");
    }
    else if (p == "gamegear") {
        candidates.push_back("gamegear");
    }
    else if (p == "gb" || p == "gameboy") {
        candidates.push_back("gameboy");
    }
    else if (p == "gbc") {
        candidates.push_back("gbcolor");
    }
    else if (p == "atari2600") {
        candidates.push_back("a2600");
    }
    else if (p == "atari5200") {
        candidates.push_back("a5200");
    }
    else if (p == "atari7800") {
        candidates.push_back("a7800");
    }
    else if (p == "atarilynx" || p == "lynx") {
        candidates.push_back("lynx");
        candidates.push_back("lynx48k");
    }
    else if (p == "atarist") {
        candidates.push_back("st");
        candidates.push_back("ste");
    }
    else if (p == "amiga") {
        candidates.push_back("a500");
        candidates.push_back("a1200");
        candidates.push_back("a2000");
    }
    else if (p == "apple2") {
        candidates.push_back("apple2ee");
        candidates.push_back("apple2");
    }
    else if (p == "psx" || p == "ps1") {
        candidates.push_back("psu");
        candidates.push_back("pse");
    }
    else if (p == "saturn") {
        candidates.push_back("saturn");
    }
    else if (p == "pcengine" || p == "pce" || p == "tg16") {
        candidates.push_back("pce");
    }
    else if (p == "colecovision") {
        candidates.push_back("coleco");
    }
    else if (p == "intellivision") {
        candidates.push_back("intv");
    }
    else if (p == "wonderswan" || p == "wswan") {
        candidates.push_back("wswan");
    }
    else if (p == "wonderswancolor" || p == "wscolor") {
        candidates.push_back("wscolor");
    }
    else if (p == "arcade" || p == "mame") {
        candidates.push_back("arcade");
    }

    return candidates;
}

std::string MameNames::queryDatabase(const std::string& platform, const std::string& mameName)
{
    if (!mDb)
        return "";

    if (!platform.empty() && mPlatformStmt) {
        sqlite3_reset(mPlatformStmt);
        sqlite3_bind_text(mPlatformStmt, 1, platform.c_str(),
                          static_cast<int>(platform.size()), SQLITE_STATIC);
        sqlite3_bind_text(mPlatformStmt, 2, mameName.c_str(),
                          static_cast<int>(mameName.size()), SQLITE_STATIC);
        if (sqlite3_step(mPlatformStmt) == SQLITE_ROW) {
            const unsigned char* text = sqlite3_column_text(mPlatformStmt, 0);
            if (text)
                return std::string(reinterpret_cast<const char*>(text));
        }
    }
    else if (platform.empty() && mShortNameStmt) {
        sqlite3_reset(mShortNameStmt);
        sqlite3_bind_text(mShortNameStmt, 1, mameName.c_str(),
                          static_cast<int>(mameName.size()), SQLITE_STATIC);
        if (sqlite3_step(mShortNameStmt) == SQLITE_ROW) {
            const unsigned char* text = sqlite3_column_text(mShortNameStmt, 0);
            if (text)
                return std::string(reinterpret_cast<const char*>(text));
        }
    }

    return "";
}

std::string MameNames::getRealName(const std::string& platform, const std::string& mameName)
{
    if (mameName.empty())
        return "";

    std::string cacheKey = platform + "\t" + mameName;
    {
        std::lock_guard<std::mutex> lock(mDbMutex);
        auto it = mCache.find(cacheKey);
        if (it != mCache.end())
            return it->second;

        // Query database with platform candidates
        std::vector<std::string> candidates = getPlatformCandidates(platform);
        for (const auto& cand : candidates) {
            std::string result = queryDatabase(cand, mameName);
            if (!result.empty()) {
                mCache[cacheKey] = result;
                return result;
            }
        }

        // Fallback: try arcade
        if (platform != "arcade" && platform != "mame") {
            std::string arcadeResult = queryDatabase("arcade", mameName);
            if (!arcadeResult.empty()) {
                mCache[cacheKey] = arcadeResult;
                return arcadeResult;
            }
        }

        // Fallback: shortname alone across any system
        std::string anyResult = queryDatabase("", mameName);
        if (!anyResult.empty()) {
            mCache[cacheKey] = anyResult;
            return anyResult;
        }

        // Fallback: legacy XML mapping
        auto xmlIt = mNamePairs.find(mameName);
        if (xmlIt != mNamePairs.end() && !xmlIt->second.empty()) {
            mCache[cacheKey] = xmlIt->second;
            return xmlIt->second;
        }

        // Not found, cache and return original mameName
        mCache[cacheKey] = mameName;
        return mameName;
    }
}

std::string MameNames::getCleanName(const std::string& platform, const std::string& mameName)
{
    static const bool stripInfo {Settings::getInstance()->getBool("MAMENameStripExtraInfo")};
    if (stripInfo)
        return Utils::String::removeParenthesis(getRealName(platform, mameName));
    else
        return getRealName(platform, mameName);
}

std::vector<std::pair<std::string, std::string>> MameNames::getGamesForPlatform(const std::string& platform)
{
    std::vector<std::pair<std::string, std::string>> games;
    if (!mDb)
        return games;

    std::vector<std::string> candidates = getPlatformCandidates(platform);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT shortname, longname FROM t WHERE platform = ?1 ORDER BY shortname";
    if (sqlite3_prepare_v2(mDb, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return games;

    std::unordered_set<std::string> seenShortnames;
    const bool stripInfo = Settings::getInstance()->getBool("MAMENameStripExtraInfo");

    for (const auto& cand : candidates) {
        sqlite3_reset(stmt);
        sqlite3_bind_text(stmt, 1, cand.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* s = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            const char* l = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            if (s && l) {
                std::string shortname = s;
                if (seenShortnames.insert(shortname).second) {
                    std::string cleanName = l;
                    if (stripInfo) {
                        cleanName = Utils::String::removeParenthesis(cleanName);
                    }
                    games.emplace_back(shortname, cleanName);
                }
            }
        }
    }

    sqlite3_finalize(stmt);
    return games;
}


