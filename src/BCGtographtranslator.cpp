/**
 * \file BCGtographtranslator.cpp
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
#include <vector>
#include <bcg_user.h>

#include "BCGtographtranslator.h"
#include "types.h"

using namespace boost;
using namespace std;

namespace kayrebt
{
	BCGToGraphTranslator::BCGToGraphTranslator(GraphType& graph, const BCG_TYPE_OBJECT_TRANSITION& bcg) : _graph(graph), _bcg(bcg)
	{
	}

	void BCGToGraphTranslator::operator()()
	{
		BCG_TYPE_STATE_NUMBER bcgSource;
		BCG_TYPE_LABEL_NUMBER bcgEdge;
		BCG_TYPE_STATE_NUMBER bcgTarget;

		BCG_OT_ITERATE_PLN(_bcg, bcgSource, bcgEdge, bcgTarget)
		{
			NodeDescriptor newNode = add_vertex(_graph);
			_graph[newNode].label = BCG_OT_LABEL_STRING(_bcg,bcgEdge);
			_graph[newNode].id = _index++;
			_succs[bcgSource].push_back(newNode);
			_targets[newNode] = bcgTarget;
		} BCG_OT_END_ITERATE;

		NodeIterator vi, vi_end;
		// Let's build all edges we have discovered
		for (tie(vi, vi_end) = vertices(_graph); vi != vi_end ; ++vi) {
			auto maybeDot = _targets.find(*vi);
			if (maybeDot == _targets.end())
				continue;
			for (NodeDescriptor n : _succs[maybeDot->second])
				add_edge(*vi,n,_graph);
		}

		// Next, we have to reenforce the invariant:
		// 1 entry node per activity diagram
		// 1 exit node per activity diagram
		NodeDescriptor init = add_vertex(_graph);
		_graph[init].id = _index++;
		_graph[init].label = "INIT";
		NodeDescriptor end = add_vertex(_graph);
		_graph[end].id = _index++;
		_graph[end].label = "END";
		for (tie(vi, vi_end) = vertices(_graph); vi != vi_end ; ++vi) {
			if (in_degree(*vi,_graph) == 0 && *vi != init) //init nodes
				add_edge(init,*vi,_graph);
			if (out_degree(*vi,_graph) == 0 && *vi != end) //ending nodes
				add_edge(*vi,end,_graph);
		}
	}

}
