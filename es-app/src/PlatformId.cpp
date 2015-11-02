#include "PlatformId.h"
#include "Log.h"

#include <string.h>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

map<string,map<string,string> > mameNameToRealName;

namespace PlatformIds
{
  void populateMameNameToRealName(const std::string &system) {
    map<string,string> nameMap;
    string filename = string("./MameMappings/") + system + string(".tsv");
    LOG(LogDebug) << "Loading short->long mapping from " << filename;
    std::ifstream infile(filename.c_str());
    string shortName, longName;
    string line;
    while (getline(infile, line)) {
      istringstream iss(line);
      getline(iss, shortName, '\t');
      getline(iss, longName, '\t');
      nameMap[shortName] = longName;
    }
    mameNameToRealName[system] = nameMap;
  }

  const std::string &getCleanMameName(const std::vector<std::string>& platformIds, const std::string &from)
	{
	  string fromString(from);
	  for (std::string s : platformIds) {
	    if (mameNameToRealName.find(s) == mameNameToRealName.end()) {
	      populateMameNameToRealName(s);
	    }
	    map<string,string> &nameMap = mameNameToRealName[s];
	    if (nameMap.find(fromString) != nameMap.end()) {
	      const std::string &filename = nameMap[fromString];
	      LOG(LogDebug) << from << " maps to " << filename;
	      return filename;
	    }
	  }
	  LOG(LogDebug) << from << " has no mapping.";
	  return from;
	}
}
