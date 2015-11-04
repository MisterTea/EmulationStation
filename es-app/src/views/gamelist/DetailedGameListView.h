#pragma once

#include "views/gamelist/BasicGameListView.h"
#include "components/ScrollableContainer.h"
#include "components/RatingComponent.h"
#include "components/DateTimeComponent.h"
#include "SystemData.h"

class DetailedGameListView : public BasicGameListView
{
public:
	DetailedGameListView(Window* window, const FileData& root, SystemData* system);

	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme) override;

	virtual void onFilesChanged() override;
	virtual void onMetaDataChanged(const FileData& file) override;

	virtual const char* getName() const override { return "detailed"; }

protected:
	virtual void launch(FileData& game) override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void updateInfoPanel() override;

	void initMDLabels();
	void initMDValues();

	ImageComponent mImage;

	TextComponent mLblRating, mLblReleaseDate, mLblDeveloper, mLblPublisher, mLblGenre, mLblPlayers, mLblLastPlayed, mLblPlayCount, mLblFavorite;

	RatingComponent mRating;
	DateTimeComponent mReleaseDate;
	TextComponent mDeveloper;
	TextComponent mPublisher;
	TextComponent mGenre;
	TextComponent mPlayers;
	DateTimeComponent mLastPlayed;
	TextComponent mPlayCount;
	TextComponent mFavorite;

	std::vector<TextComponent*> getMDLabels();
	std::vector<GuiComponent*> getMDValues();

	ScrollableContainer mDescContainer;
	TextComponent mDescription;

	SystemData* mSystem;
};
