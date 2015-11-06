#ifndef DECIDER_H
#define DECIDER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <future>
#include <mutex>

#include "mark.h"
#include "diagram_marker.h"

class Decider {
private:
	std::unordered_map<std::string,std::shared_future<Mark>> _markers;
	std::string _pathToDiagrams;
	std::mutex _mapLock;

public:
	Decider(std::string pathToDiagrams);
	std::shared_future<Mark> decide(std::string relPath);
	std::vector<std::string> getExploredDiagramsPath();
};

#endif
