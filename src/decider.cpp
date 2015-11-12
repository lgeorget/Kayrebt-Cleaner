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
#include "node_marker.h"
#include "mark.h"

Decider::Decider(std::string pathToDiagrams, std::string outputDir) :
	_pathToDiagrams(pathToDiagrams), _outputDir(outputDir)
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
					std::unique_ptr<DiagramMarker> marker(
						new DiagramMarker(*this,
							newDiagram, relPath,
							_outputDir,
							_deciderMarker
						));
					Mark result = marker->getMark();
					if (result == Mark::CALL) {
						std::unique_lock<std::mutex> lock(_printerLock);
						_diagramPrinters.push_back(std::move(marker));
					}
					unregisterThread();
					return result;
				});
		}
	}
	lock.unlock();
	return _markers[relPath];
}

void Decider::outputAllDiagrams(std::ostream& out) {
	std::unique_lock<std::mutex> lockCounter(_threadCounterLock);
	std::unique_lock<std::mutex> lockMap(_mapLock, std::defer_lock);
	_threadFinished.wait(lockCounter,
			[this,&lockMap](){
				std::cerr << "Waking up " << _threadCounter << std::endl;
				return _threadCounter <= 0 &&
				       lockMap.try_lock();
			});
	lockCounter.unlock();
	std::cerr << _markers.size() << " functions explored" << std::endl;
	for (const auto& p : _markers) {
		std::shared_future<Mark> computedMark = p.second;
		out << p.first << " : " << computedMark.get() << "\n";
	}
	std::cerr << "All marks computed" << std::endl;
	for (auto&& printer : _diagramPrinters) {
		printer->outputDiagram();
	}
	std::cerr << "All diagrams exported to the output directory" << std::endl;
	lockMap.unlock();
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
