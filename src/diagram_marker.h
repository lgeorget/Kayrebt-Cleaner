#ifndef DIAGRAM_MARKER_H
#define DIAGRAM_MARKER_H

#include <string>
#include <functional>

#include "types.h"
#include "mark.h"

class Decider;

class DiagramMarker {
private:
	Decider& _decider;
	std::string _thisDiagramPath;
	std::string _thisDiagramRelPath;
	kayrebt::GraphType _graph;
	std::function<Mark(const kayrebt::Node&)> _nodeMarker;
	bool _markComputed = false;
	Mark _mark = Mark::LAST_AND_UNUSED_MARK;

public:
	DiagramMarker(Decider& decider,
		std::string diagram, std::string relPath,
		const std::function<Mark(const kayrebt::Node&)>& nodeMarker);
	Mark getMark();
	const std::string& getRelPath() const;
};

#endif
