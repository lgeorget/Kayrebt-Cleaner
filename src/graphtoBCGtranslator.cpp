#include <iostream>
#include <boost/graph/graphviz.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graph_traits.hpp>
#include <string>
#include <map>
#include <list>
#include <utility>
#include <bcg_user.h>

#include "graphtoBCGtranslator.h"
#include "types.h"

using namespace boost;
using namespace std;

namespace kayrebt
{
	const NodeDescriptor NO_NODE = graph_traits<GraphType>::null_vertex();

	NodeDescriptor GraphToBCGTranslator::findInitNode()
	{
		NodeIterator vi, vi_end;
		NodeDescriptor init = NO_NODE;
		for (tie(vi, vi_end) = vertices(_graph); vi != vi_end && init == NO_NODE; ++vi) {
			if (_graph[*vi].label == "INIT") {
				init = *vi;
				cerr << "Found INIT node : " << _graph[*vi].id << endl;
			}
		}
		return init;
	}

	GraphToBCGTranslator::GraphToBCGTranslator(const GraphType& graph) : _graph(graph)
	{
		NodeDescriptor initNode = findInitNode();
		if (initNode == NO_NODE) { //this is not good!
			cerr << "WARNING: there is no node labelled \"INIT\" in"
				"the graph. The algorithm cannot run." << endl;
		} else {
			_nonVisited.push_back(make_pair(initNode,_index++));
		}
	}

	void GraphToBCGTranslator::operator()()
	{
		while (!_nonVisited.empty()) {
			NodeDescriptor node;
			BCG_TYPE_STATE_NUMBER dot;
			auto pDotNode = _nonVisited.front();
			_nonVisited.pop_front();
			_visited.insert(pDotNode);
			tie(node,dot) = pDotNode;

			auto its = out_edges(node,_graph);
			for (auto it = its.first ; it != its.second ; ++it) {
				cerr << "Exploring edge " << _graph[source(*it,_graph)].label << " -> " << _graph[target(*it,_graph)].label << endl;
				NodeDescriptor exitNode = target(*it,_graph);
				auto foundNode = _visited.find(exitNode);
				if (foundNode == _visited.cend()) {
					BCG_TYPE_STATE_NUMBER state = _index++;
					foundNode = _visited.emplace(exitNode,state).first;
					_nonVisited.emplace_back(exitNode,state);
				}
				BCG_IO_WRITE_BCG_EDGE(dot,const_cast<char*>(_graph[node].label.c_str()),foundNode->second);
			}
			if (its.first == its.second) { //for last node
				BCG_TYPE_STATE_NUMBER state = _index++;
				BCG_IO_WRITE_BCG_EDGE(dot,const_cast<char*>(_graph[node].label.c_str()),state);
			}
		}
	}

}
