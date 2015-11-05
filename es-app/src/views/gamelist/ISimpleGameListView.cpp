#include "views/gamelist/ISimpleGameListView.h"
#include "ThemeData.h"
#include "SystemData.h"
#include "Window.h"
#include "views/ViewController.h"
#include "Sound.h"
#include "Settings.h"
#include "GamelistDB.h"

ISimpleGameListView::ISimpleGameListView(Window* window, const FileData& root) : IGameListView(window, root),
mHeaderText(window), mHeaderImage(window), mBackground(window), mThemeExtras(window), mFavoriteChange(false)
{
	mHeaderText.setText("Logo Text");
	mHeaderText.setSize(mSize.x(), 0);
	mHeaderText.setPosition(0, 0);
	mHeaderText.setAlignment(ALIGN_CENTER);

	mHeaderImage.setResize(0, mSize.y() * 0.185f);
	mHeaderImage.setOrigin(0.5f, 0.0f);
	mHeaderImage.setPosition(mSize.x() / 2, 0);

	mBackground.setResize(mSize.x(), mSize.y());

	addChild(&mHeaderText);
	addChild(&mBackground);
	addChild(&mThemeExtras);

	mCursorStack.push(mRoot);
}

void ISimpleGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	using namespace ThemeFlags;
	mBackground.applyTheme(theme, getName(), "background", ALL);
	mHeaderImage.applyTheme(theme, getName(), "logo", ALL);
	mHeaderText.applyTheme(theme, getName(), "logoText", ALL);
	mThemeExtras.setExtras(ThemeData::makeExtras(theme, getName(), mWindow));

	if(mHeaderImage.hasImage())
	{
		removeChild(&mHeaderText);
		addChild(&mHeaderImage);
	}else{
		addChild(&mHeaderText);
		removeChild(&mHeaderImage);
	}
}

// we could be tricky here to be efficient;
// but this shouldn't happen very often so we'll just always repopulate
void ISimpleGameListView::onFilesChanged()
{
	FileData cursor = getCursor();
	FileData parent = mCursorStack.top();
	populateList(parent.getChildren());
	setCursor(cursor);
  updateInfoPanel();
}

void ISimpleGameListView::onMetaDataChanged(const FileData& file)
{
	onFilesChanged();
}

bool ISimpleGameListView::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		if(config->isMappedTo(INPUT_4B_LEFT, input))
		{
			FileData cursor = getCursor();
			if(cursor.getType() == GAME)
			{
				//Sound::getFromTheme(getTheme(), getName(), "launch")->play();
				launch(cursor);
			}else{
				// it's a folder
				if(cursor.getChildren().size() > 0)
				{
					mCursorStack.push(cursor);
					populateList(cursor.getChildren());
				}
			}

			return true;
		}else if(config->isMappedTo(INPUT_4B_DOWN, input))
		{
			if(mCursorStack.top() != mRoot)
			{
				FileData old_cursor = mCursorStack.top();
				mCursorStack.pop();
				populateList(mCursorStack.top().getChildren());
				setCursor(old_cursor);

				//Sound::getFromTheme(getTheme(), getName(), "back")->play();
			}else{
				onFocusLost();

				if (mFavoriteChange)
				{
					ViewController::get()->setInvalidGamesList(getCursor().getSystem());
					mFavoriteChange = false;
				}

				ViewController::get()->goToSystemView(mRoot.getSystem());
			}

			return true;
		}else if (config->isMappedTo(INPUT_4B_UP, input))
		{
			FileData cursor = getCursor();
			if (cursor.getSystem()->getHasFavorites())
			{
				if (cursor.getType() == GAME)
				{
					mFavoriteChange = true;
					MetaDataMap md = cursor.get_metadata();
					std::string value = md.get("favorite");
					if (value.compare("no") == 0)
					{
						md.set("favorite", "yes");
					}
					else
					{
						md.set("favorite", "no");
					}
          cursor.set_metadata(md);

          onFilesChanged();
				}
			}
		}else if(config->isMappedTo(INPUT_RIGHT, input))
		{
			if(Settings::getInstance()->getBool("QuickSystemSelect"))
			{
				onFocusLost();
				if (mFavoriteChange)
				{
					ViewController::get()->setInvalidGamesList(getCursor().getSystem());
					mFavoriteChange = false;
				}
				ViewController::get()->goToNextGameList();
				return true;
			}
		}else if(config->isMappedTo(INPUT_LEFT, input))
		{
			if(Settings::getInstance()->getBool("QuickSystemSelect"))
			{
				onFocusLost();
				if (mFavoriteChange)
				{
					ViewController::get()->setInvalidGamesList(getCursor().getSystem());
					mFavoriteChange = false;
				}
				ViewController::get()->goToPrevGameList();
				return true;
			}
		}
	}

	return IGameListView::input(config, input);
}
