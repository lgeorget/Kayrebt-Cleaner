#include <boost/config.hpp>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <future>

#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graphviz.hpp>

#include "diagram_marker.h"
#include "decider.h"
#include "mark.h"

DiagramMarker::DiagramMarker(Decider& decider, std::string diagram,
	std::string relPath, const std::function<Mark(const std::string&)>& nodeMarker) :
	_decider(decider), _thisDiagramPath(diagram), _thisDiagramRelPath(relPath),
	_nodeMarker(nodeMarker)
{
	std::string::size_type pos = _thisDiagramRelPath.find_last_of('/');
	if (pos == std::string::npos)
		_thisDiagramRelPath = "./";
	else
		_thisDiagramRelPath.erase(pos + 1);
}

DiagramMarker::~DiagramMarker() {
	//this thread has terminated all activity, unregistering
	_decider.unregisterThread();
}

Mark DiagramMarker::getMark()
{
	if (_mark != Mark::LAST_AND_UNUSED_MARK)
		return _mark;
/*
	if (_thisDiagramPath.find("fs/") == std::string::npos ||
	    _thisDiagramPath.find("read") == std::string::npos)
		return Mark::DISCARDABLE;
		*/

	boost::dynamic_properties dp_in(boost::ignore_other_properties);
	dp_in.property("label", get(&kayrebt::Node::label, _graph));
	dp_in.property("shape", get(&kayrebt::Node::shape, _graph));
	dp_in.property("type", get(&kayrebt::Node::type, _graph));
	dp_in.property("label", get(&kayrebt::Edge::condition, _graph));
	dp_in.property("URL", get(&kayrebt::Node::url, _graph));
	std::ifstream stream_graphin(_thisDiagramPath);
	try {
		boost::read_graphviz(stream_graphin, _graph, dp_in, "id");
	} catch (boost::graph_exception& e) {
	/*	std::cerr << "WARNING : no graph could be parsed for " << _thisDiagramPath
		          << "\nAssuming DISCARDABLE." << std::endl;*/
		_mark =  Mark::DISCARDABLE;
		return _mark;
	}

	std::vector<std::shared_future<Mark>> nodeMarks;

	kayrebt::NodeIterator vi,vend;
	for (std::tie(vi,vend) = boost::vertices(_graph) ; vi != vend ; ++vi) {
		if (_nodeMarker(_graph[*vi].label) != Mark::DISCARDABLE)
			_mark =  Mark::CALL;
		if (_graph[*vi].type == "call" && !_graph[*vi].url.empty()) {
			std::string path = _graph[*vi].url + ".dot";
			if (path.find_last_of('/') == 1)
				path = _thisDiagramRelPath + path.substr(2,std::string::npos);
			if (!path.empty()) {
			/*	std::cerr << "For " << _thisDiagramRelPath << " we need "
					<< path << std::endl;*/
				nodeMarks.push_back(_decider.decide(path));
			}
		} else {
			nodeMarks.emplace_back(std::async(std::launch::deferred,
			                       _nodeMarker,_graph[*vi].label));
		}
	}

	if (_mark != Mark::LAST_AND_UNUSED_MARK)
		_mark = std::any_of(nodeMarks.begin(),nodeMarks.end(),
				[](decltype(nodeMarks)::value_type& m) {
					Mark result = m.get();
					return result != Mark::DISCARDABLE &&
					       result != Mark::LAST_AND_UNUSED_MARK;
				}) ?
			Mark::CALL : Mark::DISCARDABLE;

	return _mark;
}

const std::string& DiagramMarker::getRelPath() const {
	return _thisDiagramRelPath;
}

