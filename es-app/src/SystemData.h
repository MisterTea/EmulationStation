#pragma once

#include <vector>
#include <string>
#include "FileData.h"
#include "Window.h"
#include "MetaData.h"
#include "ThemeData.h"

class SystemData
{
public:
	SystemData(const std::string& name, const std::string& fullName, const std::string& startPath, const std::vector<std::string>& extensions,
		   const std::string& command, const std::vector<std::string>& platformIds, const std::string& themeFolder);
	virtual ~SystemData();

	inline const std::string& getName() const { return mName; }
	inline const std::string& getFullName() const { return mFullName; }
	inline const std::string& getStartPath() const { return mStartPath; }
	inline const std::vector<std::string>& getExtensions() const { return mSearchExtensions; }
	inline const std::vector<std::string>& getPlatformIds() const { return mPlatformIds; }
	inline bool hasPlatformId(std::string id) const { return std::find(mPlatformIds.begin(), mPlatformIds.end(), id) != mPlatformIds.end(); }
	inline const std::string& getThemeFolder() const { return mThemeFolder; }
	inline bool getHasFavorites() const { return mHasFavorites; }
	inline const std::shared_ptr<ThemeData>& getTheme() const { return mTheme; }

	inline const FileData& getRootFolder() const { return mRoot; }

	std::string getThemePath() const;

	unsigned int getGameCount() const;
	unsigned int getFavoritesCount() const;
	bool hasFileWithImage() const;

	void launchGame(Window* window, FileData game) const;

	// Load or re-load theme.
	void loadTheme();

private:
	std::string mName;
	std::string mFullName;
	std::string mStartPath;
	std::vector<std::string> mSearchExtensions;
	std::string mLaunchCommand;
	std::vector<std::string> mPlatformIds;
	std::string mThemeFolder;
	std::shared_ptr<ThemeData> mTheme;
  bool mHasFavorites;

	FileData mRoot;
};
