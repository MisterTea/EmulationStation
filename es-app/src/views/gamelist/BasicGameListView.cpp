#include "views/gamelist/BasicGameListView.h"
#include "views/ViewController.h"
#include "Renderer.h"
#include "Window.h"
#include "ThemeData.h"
#include "SystemData.h"
#include "Settings.h"

BasicGameListView::BasicGameListView(Window* window, const FileData& root)
	: ISimpleGameListView(window, root), mList(window)
{
	mList.setSize(mSize.x(), mSize.y() * 0.8f);
	mList.setPosition(0, mSize.y() * 0.2f);
	addChild(&mList);

	populateList(mCursorStack.top().getChildren());
}

void BasicGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	ISimpleGameListView::onThemeChanged(theme);
	using namespace ThemeFlags;
	mList.applyTheme(theme, getName(), "gamelist", ALL);
}

void BasicGameListView::onFilesChanged()
{
	// might switch to a detailed view
	ViewController::get()->reloadGameListView(this);
}

void BasicGameListView::onMetaDataChanged(const FileData& file)
{
	// might switch to a detailed view
	ViewController::get()->reloadGameListView(this);
}

void BasicGameListView::populateList(const std::vector<FileData>& files)
{
	mList.clear();

	mHeaderText.setText(mRoot.getSystem()->getFullName());

	bool hasFavorites = false;

	if (Settings::getInstance()->getBool("FavoritesOnly"))
	{
	for(auto it = files.begin(); it != files.end(); it++)
	{
			if (it->getType() == GAME)
			{
				if (it->get_metadata().get("favorite").compare("yes") == 0)
				{
					hasFavorites = true;
					break;
				}
			}
		}
	}

	// The TextListComponent would be able to insert at a specific position,
	// but the cost of this operation could be seriously huge.
	// This naive implemention of doing a first pass in the list is used instead.
	if(! Settings::getInstance()->getBool("FavoritesOnly")){
		for(auto it = files.begin(); it != files.end(); it++)
		{
			if (it->getType() != FOLDER && it->get_metadata().get("favorite").compare("yes") == 0)
			{
				mList.add("\uF006 " + it->getName(), *it, (it->getType() == FOLDER)); // FIXME Folder as favorite ?
			}
		}
	}

	for(auto it = files.begin(); it != files.end(); it++)
	{
		if (Settings::getInstance()->getBool("FavoritesOnly") && hasFavorites)
		{
			if (it->getType() == GAME)
			{
				if (it->get_metadata().get("favorite").compare("yes") == 0)
				{
		mList.add(it->getName(), *it, (it->getType() == FOLDER));
	}
}
		}
		else
		{
			mList.add(it->getName(), *it, (it->getType() == FOLDER));
		}
	}
}

const FileData& BasicGameListView::getCursor()
{
	return mList.getSelected();
}

void BasicGameListView::setCursor(const FileData& cursor)
{
	if(!mList.setCursor(cursor))
	{
		LOG(LogWarning) << "Tried to setCursor to non-present filedata (" << cursor.getFileID() << " for system " << cursor.getSystemID() << ")";
	}
}

void BasicGameListView::launch(FileData& game)
{
	ViewController::get()->launch(game);
}

std::vector<HelpPrompt> BasicGameListView::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;

	if(Settings::getInstance()->getBool("QuickSystemSelect"))
		prompts.push_back(HelpPrompt("left/right", "system"));
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt(inputCategoryToString(INPUT_4B_LEFT), "launch"));
	prompts.push_back(HelpPrompt(inputCategoryToString(INPUT_4B_DOWN), "back"));
	prompts.push_back(HelpPrompt(inputCategoryToString(INPUT_SELECT), "options"));
	return prompts;
}
