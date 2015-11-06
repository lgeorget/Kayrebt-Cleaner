#ifndef DECIDER_H
#define DECIDER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <future>
#include <mutex>
#include <regex>

#include "mark.h"
#include "diagram_marker.h"

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
	Mark operator()(const std::string& symbol) const;
};

class Decider {
private:
	std::map<std::string,std::shared_future<Mark>> _markers;
	std::string _pathToDiagrams;
	std::mutex _mapLock;
	std::mutex _threadCounterLock;
	int _threadCounter = 0;
	std::condition_variable _threadFinished;
	const NodeMarker _deciderMarker;

public:
	Decider(std::string pathToDiagrams);
	std::shared_future<Mark> decide(std::string relPath);
	void registerNewThread();
	void unregisterThread();

	friend std::ostream& operator<<(std::ostream& out, Decider& d);
};

std::ostream& operator<<(std::ostream& out, Decider& d);
#endif
