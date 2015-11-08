#include <boost/config.hpp>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <future>
#include <map>
#include <iterator>

#include <boost/property_map/property_map.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/filesystem.hpp>

#include "diagram_marker.h"
#include "decider.h"
#include "mark.h"

DiagramMarker::DiagramMarker(Decider& decider, std::string diagram,
	std::string relPath, const std::string& outputDir,
	const NodeMarker& nodeMarker) :
	_decider(decider), _thisDiagramPath(diagram), _thisDiagramRelPath(relPath),
	_outputDir(outputDir), _nodeMarker(nodeMarker),
	_dp(boost::ignore_other_properties)//,
//	_gfile(get_property(_graph,&kayrebt::GraphAttr::file)),
//	_gline(get_property(_graph,&kayrebt::GraphAttr::line))
{
	std::string::size_type pos = _thisDiagramRelPath.find_last_of('/');
	if (pos == std::string::npos)
		_thisDiagramRelDir = "./";
	else
		_thisDiagramRelDir = _thisDiagramRelPath.substr(0,pos+1);

	_dp.property("id",    get(&kayrebt::Node::id, _graph));
	_dp.property("label", get(&kayrebt::Node::label, _graph));
	_dp.property("shape", get(&kayrebt::Node::shape, _graph));
	_dp.property("type", get(&kayrebt::Node::type, _graph));
	_dp.property("label", get(&kayrebt::Edge::condition, _graph));
	_dp.property("URL", get(&kayrebt::Node::url, _graph));
	_dp.property("line", get(&kayrebt::Node::line, _graph));
//	_dp.property("file",  _gfile);
//	_dp.property("line",  _gline);
}

Mark DiagramMarker::getMark()
{
	if (_mark != Mark::LAST_AND_UNUSED_MARK)
		return _mark;

	std::ifstream stream_graphin(_thisDiagramPath);
	try {
		boost::read_graphviz(stream_graphin, _graph, _dp, "id");
	} catch (boost::graph_exception& e) {
	/*	std::cerr << "WARNING : no graph could be parsed for " << _thisDiagramPath
		          << "\nAssuming DISCARDABLE." << std::endl;*/
		_mark =  Mark::DISCARDABLE;
		return _mark;
	}

	kayrebt::NodeIterator vi,vend;
	for (std::tie(vi,vend) = boost::vertices(_graph) ; vi != vend ; ++vi) {
		//Decide the mark with as few info as early as possible
		Mark markFromLabel = _nodeMarker(_graph[*vi].label);
		if (markFromLabel != Mark::DISCARDABLE &&
		    markFromLabel != Mark::LAST_AND_UNUSED_MARK)
			_mark =  Mark::CALL;

		if (_graph[*vi].type == "call" && !_graph[*vi].url.empty()) {
			std::string path = _graph[*vi].url + ".dot";
			if (path.find_last_of('/') == 1)
				path = _thisDiagramRelDir + path.substr(2,std::string::npos);
			if (!path.empty()) {
			/*	std::cerr << "For " << _thisDiagramRelPath << " we need "
					<< path << std::endl;*/
				_nodeMarks[*vi] = _decider.decide(path);
			}
		} else {
			_nodeMarks.emplace(*vi, std::async(std::launch::deferred,
				[this,&vi,markFromLabel]() {
					//default nodes to DISCARDABLE
					return markFromLabel ==
					       Mark::LAST_AND_UNUSED_MARK ?
					       Mark::DISCARDABLE : markFromLabel;
				})
			);
		}
	}

	// If we could not decide a mark yet, unpack as few future<Mark> as
	// possible in order to decide
	// Start filling in _actualMarks in the process
	if (_mark == Mark::LAST_AND_UNUSED_MARK) {
		_mark = std::any_of(_nodeMarks.cbegin(),_nodeMarks.cend(),
				    [this](const decltype(_nodeMarks)::value_type& p) {
				    	Mark m = p.second.get();
				    	_actualMarks[p.first] = m;
				    	return m != Mark::LAST_AND_UNUSED_MARK &&
				    	       m != Mark::DISCARDABLE;
				    	}) ?
			Mark::CALL : Mark::DISCARDABLE;
	}

	return _mark;
}

void DiagramMarker::outputDiagram() {
	// we must make sure that we don't call get() again on values already
	// acquired from the future<Mark>, so we need something a little more
	// sophisticated than std::transform(_nodeMarks,_actualMarks,get())
	for (const auto& p : _nodeMarks) {
		if (_actualMarks.find(p.first) == _actualMarks.cend())
			_actualMarks[p.first] = p.second.get();
	}

	boost::filesystem::create_directories(_outputDir + "/" + _thisDiagramRelDir);
	std::ofstream stream_graphout(_outputDir + "/" + _thisDiagramRelPath);
	_dp.property("mark",
			boost::make_assoc_property_map(_actualMarks));

	try {
		boost::write_graphviz_dp(stream_graphout, _graph, _dp, "id");
	} catch (std::exception& e) {
		std::cerr << "Cannot output graph " << _thisDiagramRelPath << std::endl;
	}
}

const std::string& DiagramMarker::getRelDir() const {
	return _thisDiagramRelDir;
}

