#ifndef DIAGRAM_MARKER_H
#define DIAGRAM_MARKER_H

#include <boost/config.hpp>
#include <string>
#include <functional>
#include <map>
#include <future>

#include <boost/property_map/property_map.hpp>
#include <boost/property_map/dynamic_property_map.hpp>

#include "types.h"
#include "node_marker.h"
#include "mark.h"

class Decider;

class DiagramMarker {
private:
	Decider& _decider;
	std::string _thisDiagramPath;
	std::string _thisDiagramRelDir;
	std::string _thisDiagramRelPath;
	kayrebt::GraphType _graph;
	const NodeMarker& _nodeMarker;
	Mark _mark = Mark::LAST_AND_UNUSED_MARK;
	boost::dynamic_properties _dp;
//	boost::ref_property_map<kayrebt::GraphType*,std::string> _gfile;
//	boost::ref_property_map<kayrebt::GraphType*,unsigned int> _gline;
	std::map<kayrebt::NodeDescriptor, std::shared_future<Mark>> _nodeMarks;
	std::map<kayrebt::NodeDescriptor, Mark> _actualMarks;

public:
	DiagramMarker(Decider& decider,
		std::string diagram, std::string relPath,
		const NodeMarker& nodeMarker);
	Mark getMark();
	const std::string& getRelDir() const;
	void outputDiagram();
};

#endif
