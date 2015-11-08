#ifndef DECIDER_H
#define DECIDER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <future>
#include <mutex>
#include <regex>
#include <memory>

#include "node_marker.h"
#include "mark.h"
#include "diagram_marker.h"

class Decider {
private:
	std::map<std::string,std::shared_future<Mark>> _markers;
	std::string _pathToDiagrams;
	std::mutex _mapLock;
	std::mutex _threadCounterLock;
	int _threadCounter = 0;
	std::condition_variable _threadFinished;
	const NodeMarker _deciderMarker;
	std::vector<std::unique_ptr<DiagramMarker>> _diagramPrinters;
	std::string _outputDir;

public:
	Decider(std::string pathToDiagrams, std::string outputDir);
	std::shared_future<Mark> decide(std::string relPath);
	void registerNewThread();
	void unregisterThread();
	void outputAllDiagrams(std::ostream& out);
};

#endif
