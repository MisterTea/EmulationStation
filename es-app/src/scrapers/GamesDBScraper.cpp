#include "scrapers/GamesDBScraper.h"
#include "Log.h"
#include "pugixml/pugixml.hpp"
#include "MetaData.h"
#include "Settings.h"
#include "Util.h"
#include <boost/assign.hpp>

const std::map<std::string, std::string> gamesdb_platformid_map = boost::assign::map_list_of
  ("3d0", "3DO")
  ("a1000", "Amiga")
  ("a1000n", "Amiga")
  ("a1200", "Amiga")
  ("a1200n", "Amiga")
  ("a2000", "Amiga")
  ("a2000n", "Amiga")
  ("a3000", "Amiga")
  ("a3000n", "Amiga")
  ("a4000", "Amiga")
  ("a400030", "Amiga")
  ("a400030n", "Amiga")
  ("a4000n", "Amiga")
  ("a4000t", "Amiga")
  ("a4000tn", "Amiga")
  ("a500", "Amiga")
  ("a500n", "Amiga")
  ("a500p", "Amiga")
  ("a500pn", "Amiga")
  ("a600", "Amiga")
  ("a600n", "Amiga")
  ("cd32", "Amiga")
  ("cd32n", "Amiga")
  ("cpc464", "Amstrad CPC")
  ("cpc464p", "Amstrad CPC")
  ("cpc6128", "Amstrad CPC")
  ("cpc6128f", "Amstrad CPC")
  ("cpc6128p", "Amstrad CPC")
  ("cpc6128s", "Amstrad CPC")
  ("cpc664", "Amstrad CPC")
  ("gx4000", "Amstrad CPC")
  ("pc20", "Amstrad CPC")
  ("pc2086", "Amstrad CPC")
  ("pc2386", "Amstrad CPC")
  ("pc3086", "Amstrad CPC")
  ("ppc512", "Amstrad CPC")
  ("ppc640", "Amstrad CPC")
  ("arcade", "Arcade")
  ("a2600", "Atari 2600")
  ("a2600p", "Atari 2600")
  ("a5200", "Atari 5200")
  ("a7800", "Atari 7800")
  ("a7800p", "Atari 7800")
  ("jaguar", "Atari Jaguar")
  ("jaguarcd", "Atari Jaguar CD")
  ("lynx", "Atari Lynx")
  ("lynx128k", "Atari Lynx")
  ("lynx48k", "Atari Lynx")
  ("lynx96k", "Atari Lynx")
  ("a130xe", "Atari XE")
  ("a65xe", "Atari XE")
  ("a65xea", "Atari XE")
  ("a800xe", "Atari XE")
  ("coleco", "Colecovision")
  ("colecop", "Colecovision")
  ("c264", "Commodore 64")
  ("c64", "Commodore 64")
  ("c64_jp", "Commodore 64")
  ("c64_se", "Commodore 64")
  ("c64c", "Commodore 64")
  ("c64c_es", "Commodore 64")
  ("c64c_se", "Commodore 64")
  ("c64cp", "Commodore 64")
  ("c64dtv", "Commodore 64")
  ("c64dx", "Commodore 64")
  ("c64g", "Commodore 64")
  ("c64gs", "Commodore 64")
  ("c64p", "Commodore 64")
  ("v364", "Commodore 64")
  ("intv", "Intellivision")
  ("intv2", "Intellivision")
  ("intvecs", "Intellivision")
  ("intvkbd", "Intellivision")
  ("intvoice", "Intellivision")
  ("neogeo", "NeoGeo")
  ("ngp", "Neo Geo Pocket")
  ("ngpc", "Neo Geo Pocket Color")
  ("n64", "Nintendo 64")
  ("nes", "Nintendo Entertainment System (NES)")
  ("nespal", "Nintendo Entertainment System (NES)")
  ("gameboy", "Nintendo Game Boy")
  ("gba", "Nintendo Game Boy Advance")
  ("gbcolor", "Nintendo Game Boy Color")
  ("32xe", "Sega 32X")
  ("32xj", "Sega 32X")
  ("segacd", "Sega CD")
  ("segacd2", "Sega CD")
  ("dc", "Sega Dreamcast")
  ("dceu", "Sega Dreamcast")
  ("dcjp", "Sega Dreamcast")
  ("gamegear", "Sega Game Gear")
  ("gamegeaj", "Sega Game Gear")
  ("genesis", "Sega Genesis")
  ("genesisp", "Sega Genesis")
  ("sms", "Sega Master System")
  ("smspal", "Sega Master System")
  ("sms1", "Sega Master System")
  ("sms1pal", "Sega Master System")
  ("smsj", "Sega Master System")
  ("megadriv", "Sega Mega Drive")
  ("megadrij", "Sega Mega Drive")
  ("saturn", "Sega Saturn")
  ("saturneu", "Sega Saturn")
  ("saturnjp", "Sega Saturn")
  ("psa", "Sony Playstation")
  ("pse", "Sony Playstation")
  ("psj", "Sony Playstation")
  ("psu", "Sony Playstation")
  ("snes", "Super Nintendo (SNES)")
  ("snespal", "Super Nintendo (SNES)")
  ("tg16", "TurboGrafx 16")
  ("pce", "TurboGrafx 16")
  ("wswan", "WonderSwan")
  ("wscolor", "WonderSwan Color")
  ("spectrum", "Sinclair ZX Spectrum");

void thegamesdb_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests,
	std::vector<ScraperSearchResult>& results)
{
	std::string path = "thegamesdb.net/api/GetGame.php?";

	std::string cleanName = params.nameOverride;
	if(cleanName.empty())
		cleanName = params.game.getCleanName();

	path += "name=" + HttpReq::urlEncode(cleanName);

	if(params.system->getPlatformIds().empty())
	{
		// no platform specified, we're done
		requests.push(std::unique_ptr<ScraperRequest>(new TheGamesDBRequest(results, path)));
	}else{
		// go through the list, we need to split this into multiple requests
		// because TheGamesDB API either sucks or I don't know how to use it properly...
		std::string urlBase = path;
		auto& platforms = params.system->getPlatformIds();
		std::vector<std::string> systemList;
		systemList.push_back("arcade");
		for(auto platformIt = platforms.begin(); platformIt != platforms.end(); platformIt++)
		{
			path = urlBase;
			auto mapIt = gamesdb_platformid_map.find(*platformIt);
			if(mapIt != gamesdb_platformid_map.end())
			{
				path += "&platform=";
				path += HttpReq::urlEncode(mapIt->second);
			}else{
			  LOG(LogWarning) << "TheGamesDB scraper warning - no support for platform " << (*platformIt);
			}

			LOG(LogDebug) << "Making Request: " << path;
			requests.push(std::unique_ptr<ScraperRequest>(new TheGamesDBRequest(results, path)));
		}
	}
}

void TheGamesDBRequest::process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results)
{
	assert(req->status() == HttpReq::REQ_SUCCESS);

	pugi::xml_document doc;
	pugi::xml_parse_result parseResult = doc.load(req->getContent().c_str());
	if(!parseResult)
	{
		std::stringstream ss;
		ss << "GamesDBRequest - Error parsing XML. \n\t" << parseResult.description() << "";
		std::string err = ss.str();
		setError(err);
		LOG(LogError) << err;
		return;
	}

	pugi::xml_node data = doc.child("Data");

	std::string baseImageUrl = data.child("baseImgUrl").text().get();

	pugi::xml_node game = data.child("Game");
	while(game && results.size() < MAX_SCRAPER_RESULTS)
	{
		ScraperSearchResult result;

		result.metadata.set("name", game.child("GameTitle").text().get());
		result.metadata.set("desc", game.child("Overview").text().get());

		boost::posix_time::ptime rd = string_to_ptime(game.child("ReleaseDate").text().get(), "%m/%d/%Y");
		result.metadata.set("releasedate", rd);

		result.metadata.set("developer", game.child("Developer").text().get());
		result.metadata.set("publisher", game.child("Publisher").text().get());
		result.metadata.set("genre", game.child("Genres").first_child().text().get());
		result.metadata.set("players", game.child("Players").text().get());

		if(Settings::getInstance()->getBool("ScrapeRatings") && game.child("Rating"))
		{
			float ratingVal = (game.child("Rating").text().as_int() / 10.0f);
			result.metadata.set("rating", ratingVal);
		}

		pugi::xml_node images = game.child("Images");

		if(images)
		{
			pugi::xml_node art = images.find_child_by_attribute("boxart", "side", "front");

			if(art)
			{
				result.thumbnailUrl = baseImageUrl + art.attribute("thumb").as_string();
				result.imageUrl = baseImageUrl + art.text().get();
			}
		}

		results.push_back(result);
		game = game.next_sibling("Game");
	}
}
