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
			auto nodeIt = _nodes.find(bcgTarget);
			if (nodeIt == _nodes.cend())
				nodeIt = _nodes.emplace(bcgTarget,add_vertex(_graph)).first;
			auto node = nodeIt->second;
			_graph[node].id = bcgTarget;
			std::string lab(BCG_OT_LABEL_STRING(_bcg,bcgEdge));
			if (lab == "i")
				_graph[node].shape = "diamond";
			else
				_graph[node].label = std::move(lab);


			if (bcgSource != BCG_OT_INITIAL_STATE(_bcg)) {
				auto predIt = _nodes.find(bcgSource);
				if (predIt == _nodes.cend())
					predIt = _nodes.emplace(bcgSource,add_vertex(_graph)).first;
				auto pred = predIt->second;
				add_edge(pred,node,_graph);
			}
		} BCG_OT_END_ITERATE;
	}

}
