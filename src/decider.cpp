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

Decider::Decider(std::string pathToDiagrams) : _pathToDiagrams(pathToDiagrams)
{}

std::shared_future<Mark> Decider::decide(std::string relPath)
{
//	std::cerr << "Deciding " << relPath << std::endl;
	std::unique_lock<std::mutex> lock(_mapLock);
	std::shared_future<Mark> futureMark = _markers[relPath];
	// If the mark has not been decided yet, decide it asynchronously
	if (!futureMark.valid()) {
		Mark m = _deciderMarker(relPath);
		if (m != Mark::LAST_AND_UNUSED_MARK) {
			_markers[relPath] = std::async(std::launch::deferred,
					               [m]() { return m; }
					    );
		} else {
			registerNewThread();
			_markers[relPath] = std::async(std::launch::async,
				[&,relPath]() {
					std::string newDiagram = _pathToDiagrams
					                       + relPath;
					DiagramMarker marker(*this, newDiagram,
						relPath,
						std::function<Mark(const std::string&)>(_deciderMarker)
						);
					return marker.getMark();
				});
		}
	}
	lock.unlock();
	return _markers[relPath];
}

std::ostream& operator<<(std::ostream& out, Decider& d) {
	std::unique_lock<std::mutex> lockCounter(d._threadCounterLock);
	std::unique_lock<std::mutex> lockMap(d._mapLock, std::defer_lock);
	d._threadFinished.wait(lockCounter,
			[&d,&lockMap](){
				return d._threadCounter <= 0 &&
				       lockMap.try_lock();
			});
	lockCounter.unlock();
	std::cerr << d._markers.size() << " functions explored" << std::endl;
	for (const auto& p : d._markers) {
		out << p.first << " : " << p.second.get() << "\n";
	}
	lockMap.unlock();
	return out;
}

void Decider::registerNewThread() {
	std::unique_lock<std::mutex> lockCounter(_threadCounterLock);
	_threadCounter++;
	lockCounter.unlock();
}

void Decider::unregisterThread() {
	std::unique_lock<std::mutex> lockCounter(_threadCounterLock);
	_threadCounter--;
	if (_threadCounter <= 0) {
		_threadCounter = 0;
		_threadFinished.notify_all();
	}
	lockCounter.unlock();
}
