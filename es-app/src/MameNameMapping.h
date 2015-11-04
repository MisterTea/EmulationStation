#pragma once

#include <map>
#include <vector>
#include <string>

namespace MameNameMapping
{
  std::string shortToFullName(const std::vector<std::string>& platformIds, const std::string &shortName);
};
