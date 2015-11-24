#include "guis/GuiScraperStart.h"
#include "guis/GuiScraperMulti.h"
#include "guis/GuiMsgBox.h"
#include "views/ViewController.h"
#include "SystemManager.h"

#include "components/TextComponent.h"
#include "components/OptionListComponent.h"
#include "components/SwitchComponent.h"

GuiScraperStart::GuiScraperStart(Window* window) : GuiComponent(window),
	mMenu(window, "SCRAPE NOW")
{
	addChild(&mMenu);

	// add filters (with first one selected)
	mFilters = std::make_shared< OptionListComponent<GameFilterFunc> >(mWindow, "SCRAPE THESE GAMES", false);
	mFilters->add("All Games",
		[](SystemData*, const FileData&) -> bool { return true; }, false);
	mFilters->add("Only missing image",
		[](SystemData*, const FileData& g) -> bool { return g.get_metadata().get("image").empty(); }, true);
	mMenu.addWithLabel("Filter", mFilters);

	//add systems (all with a platformid specified selected)
	mSystems = std::make_shared< OptionListComponent<SystemData*> >(mWindow, "SCRAPE THESE SYSTEMS", true);
	for(auto it = SystemManager::getInstance()->getSystems().begin(); it != SystemManager::getInstance()->getSystems().end(); it++)
	{
		if(!(*it)->hasPlatformId("ignore"))
			mSystems->add((*it)->getFullName(), *it, !(*it)->getPlatformIds().empty());
	}
	mMenu.addWithLabel("Systems", mSystems);

	mApproveResults = std::make_shared<SwitchComponent>(mWindow);
	mApproveResults->setState(false);
	mMenu.addWithLabel("User decides on conflicts", mApproveResults);

	mMenu.addButton("START", "start", std::bind(&GuiScraperStart::pressedStart, this));
	mMenu.addButton("BACK", "back", [&] { delete this; });

	mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

void GuiScraperStart::pressedStart()
{
	std::vector<SystemData*> sys = mSystems->getSelectedObjects();
	for(auto it = sys.begin(); it != sys.end(); it++)
	{
		if((*it)->getPlatformIds().empty())
		{
			mWindow->pushGui(new GuiMsgBox(mWindow,
				strToUpper("Warning: some of your selected systems do not have a platform set. Results may be even more inaccurate than usual!\nContinue anyway?"),
				"YES", std::bind(&GuiScraperStart::start, this),
				"NO", nullptr));
			return;
		}
	}

	start();
}

void GuiScraperStart::start()
{
	std::queue<ScraperSearchParams> searches = getSearches(mSystems->getSelectedObjects(), mFilters->getSelected());

	if(searches.empty())
	{
		mWindow->pushGui(new GuiMsgBox(mWindow,
			"NO GAMES FIT THAT CRITERIA."));
	}else{
		GuiScraperMulti* gsm = new GuiScraperMulti(mWindow, searches, mApproveResults->getState());
		mWindow->pushGui(gsm);
		delete this;
	}
}

std::queue<ScraperSearchParams> GuiScraperStart::getSearches(std::vector<SystemData*> systems, GameFilterFunc selector)
{
	std::queue<ScraperSearchParams> queue;
	for(auto sys = systems.begin(); sys != systems.end(); sys++)
	{
		std::vector<FileData> games = (*sys)->getRootFolder().getChildrenRecursive(false);
		for(auto game = games.begin(); game != games.end(); game++)
		{
			if(selector((*sys), (*game)))
			{
				ScraperSearchParams search(*sys, *game);
				queue.push(search);
			}
		}
	}

	return queue;
}

bool GuiScraperStart::input(InputConfig* config, Input input)
{
	bool consumed = GuiComponent::input(config, input);
	if(consumed)
		return true;

	if(input.value != 0 && config->isMappedTo(INPUT_4B_DOWN, input))
	{
		delete this;
		return true;
	}

	if(config->isMappedTo(INPUT_START, input) && input.value != 0)
	{
		// close everything
		Window* window = mWindow;
		while(window->peekGui() && window->peekGui() != ViewController::get())
			delete window->peekGui();
	}


	return false;
}

std::vector<HelpPrompt> GuiScraperStart::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt(inputCategoryToString(INPUT_4B_DOWN), "back"));
	prompts.push_back(HelpPrompt(inputCategoryToString(INPUT_START), "close"));
	return prompts;
}
