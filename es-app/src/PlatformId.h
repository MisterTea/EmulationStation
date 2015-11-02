#pragma once

#include <map>
#include <vector>
#include <string>

namespace PlatformIds
{
  const std::string &getCleanMameName(const std::vector<std::string>& platformIds, const std::string &from);
}
