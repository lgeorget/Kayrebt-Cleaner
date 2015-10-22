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
#include <regex>
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

		unsigned int index = 0;
		BCG_OT_ITERATE_PLN(_bcg, bcgSource, bcgEdge, bcgTarget)
		{
			std::string lab(BCG_OT_LABEL_STRING(_bcg,bcgEdge));
			NodeDescriptor node;
			if (lab == "i") {
				node = add_vertex(_graph);
				_graph[node].id = index++;
				_graph[node].shape = "diamond";
				_graph[node].label = std::string();
			} else {
				std::regex labelRegex(R"#((\d+) !"([^\"]*)")#");
				std::smatch pieces;
				if (std::regex_match(lab,pieces,labelRegex)) {
					unsigned int index = std::stoul(pieces.str(1));
					auto it = _nodes.find(index);
					if (it == _nodes.cend()) {
						node = _nodes.insert(std::make_pair(index,add_vertex(_graph))).first->second;
						_graph[node].id = index++;
						_graph[node].shape = "ellipse";
						_graph[node].label = std::move(pieces.str(2));
					} else {
						node = it->second;
					}
				} else {
					std::cerr << "Unmatched label !\n\t" << lab << std::endl;
					node = add_vertex(_graph);
					_graph[node].id = index++;
					_graph[node].shape = "ellipse";
					_graph[node].label = std::move(lab);
				}
			}

			_targets.emplace_back(bcgSource,node);
			_sources[bcgTarget].push_back(node);
		} BCG_OT_END_ITERATE;

		for (const auto& targetpair : _targets)
			for (const auto& nodesource : _sources[targetpair.first])
				add_edge(nodesource,targetpair.second,_graph);
	}

}
