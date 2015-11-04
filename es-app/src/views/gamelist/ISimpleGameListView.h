#pragma once

#include "views/gamelist/IGameListView.h"

#include "components/TextComponent.h"
#include "components/ImageComponent.h"

class ISimpleGameListView : public IGameListView
{
public:
	ISimpleGameListView(Window* window, const FileData& root);
	virtual ~ISimpleGameListView() {}

	virtual void onFilesChanged() override;
	virtual void onMetaDataChanged(const FileData& file) override;

	// Called whenever the theme changes.
	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme) override;

	virtual bool input(InputConfig* config, Input input) override;

	virtual inline void updateInfoPanel() override {}

	virtual inline void populateList(const std::vector<FileData>& files) override {}

protected:
	virtual void launch(FileData& game) = 0;

	TextComponent mHeaderText;
	ImageComponent mHeaderImage;
	ImageComponent mBackground;

	ThemeExtras mThemeExtras;

	std::stack<FileData> mCursorStack;

private:
   bool mFavoriteChange;
};
