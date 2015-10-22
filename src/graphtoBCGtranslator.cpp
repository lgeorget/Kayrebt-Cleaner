/**
 * \file graphToBCGtranslator.cpp
 * \author Laurent Georget
 * \date 2015-10-15
 * \brief
 */
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
	GraphToBCGTranslator::GraphToBCGTranslator(const GraphType& graph) : _graph(graph)
	{
	}

	void GraphToBCGTranslator::operator()()
	{
		NodeIterator vi,vi_end;
		for (tie(vi,vi_end) = vertices(_graph) ; vi != vi_end ; ++vi)
		{
			auto state = _states.find(*vi);
			if (state == _states.cend())
				state = _states.emplace(*vi,_index++).first;
			PredecessorIterator pred,pred_end;
			for (tie(pred,pred_end) = inv_adjacent_vertices(*vi,_graph) ; pred != pred_end ; ++pred)
			{
				auto predState = _states.find(*pred);
				if (predState == _states.cend())
					predState = _states.emplace(*pred,_index++).first;
				if (_graph[*vi].label.empty() ||
				    (_graph[*vi].type != "call" &&
				     _graph[*vi].type != "init" &&
				     _graph[*vi].type != "end_of_activity")) {
					writeEdge(predState->second,"i",state->second);
				} else {
					std::string label = std::to_string(_graph[*vi].id) + " !\"" + _graph[*vi].label + "\"";
					writeEdge(predState->second,label,state->second);
				}
			}

			if (in_degree(*vi,_graph) == 0)
				_initNode = *vi; //should better be unique!
		}
		std::string label = std::to_string(_graph[_initNode].id) + " !\"" + _graph[_initNode].label + "\"";
		writeEdge(0, label, _states[_initNode]);

	}
}
