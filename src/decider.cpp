#include <iostream>
#include <vector>
#include <string>
#include <future>
#include <thread>
#include <vector>
#include <algorithm>
#include <utility>
#include <functional>
#include <regex>
#include <chrono>

#include "decider.h"
#include "mark.h"

namespace {

class NodeMarker {
private:
	const std::vector<std::pair<std::regex,Mark>> _matcher{
		{std::regex("mutex.*"),                   Mark::LOCK},
		{std::regex("spin.*lock"),                Mark::LOCK},
		{std::regex("atomic_dec_and_lock.*lock"), Mark::LOCK},
		{std::regex("security_.*"),               Mark::LSM_HOOK},
		{std::regex("file->f_op->read"),          Mark::FLOW_STMT},
		{std::regex("new_sync_read"),             Mark::FLOW_STMT},
	};
public:
	Mark operator()(const kayrebt::Node& n) {
		auto match = std::find_if(_matcher.cbegin(),_matcher.cend(),
				[&n](const std::pair<std::regex,Mark>& p){
					return std::regex_search(n.label,p.first);
				});
		if (match != _matcher.cend())
			return match->second;
		else
			return Mark::DISCARDABLE;
	}
};

}

Decider::Decider(std::string pathToDiagrams) : _pathToDiagrams(pathToDiagrams)
{}

std::shared_future<Mark> Decider::decide(std::string relPath)
{
	std::cerr << "Deciding " << relPath << std::endl;
	std::unique_lock<std::mutex> lock(_mapLock);
	std::shared_future<Mark> futureMark = _markers[relPath];
	// If the mark has not been decided yet, decide it asynchronously
	if (!futureMark.valid()) {
		_markers[relPath] = std::async(std::launch::deferred, [&,relPath]() {
			std::string newDiagram = _pathToDiagrams + relPath;
			DiagramMarker marker(*this, newDiagram, relPath,
				std::function<Mark(const kayrebt::Node&)>(NodeMarker())
			);
			return marker.getMark();
		});
	}
	lock.unlock();
	return _markers[relPath];
}

std::vector<std::string> Decider::getExploredDiagramsPath() {
	std::vector<std::string> result;
	std::cout << _markers.size() << " functions explored" << std::endl;
	std::transform(_markers.cbegin(), _markers.cend(),
			std::back_inserter(result),
			[](const decltype(_markers)::value_type& p){
				//p.second.wait();
				return p.first;
			});
	return result;
}
