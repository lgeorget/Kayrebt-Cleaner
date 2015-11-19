#include <string>
#include <regex>
#include "algorithm"

#include "node_marker.h"
#include "mark.h"


Mark NodeMarker::operator()(const std::string& symbol) const {
	auto match = std::find_if(_matcher.cbegin(),_matcher.cend(),
			[&symbol](const std::pair<std::regex,Mark>& p){
			return std::regex_search(symbol,p.first);
			});
	if (match != _matcher.cend())
		return match->second;
	else
		return Mark::LAST_AND_UNUSED_MARK;
}


