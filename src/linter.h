/**
 * \file includer.h
 * \author Laurent Georget
 * \date 2015-10-23
 * \brief
 */
#include <boost/config.hpp>
#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <utility>
#include <algorithm>
#include <iterator>

#include <boost/graph/graph_utility.hpp>
#include <boost/graph/copy.hpp>
#include <boost/property_map/property_map.hpp>

namespace boost
{
	template<typename InputGraph, typename KeepPredicate>
	void lint(InputGraph& gin, const KeepPredicate& toKeep) {

		using InputGraphTraits = graph_traits<InputGraph>;
		using InputVertex = typename InputGraphTraits::vertex_descriptor;
		using InputVertexIterator = typename InputGraphTraits::vertex_iterator;
		using SuccessorIterator = typename InputGraphTraits::adjacency_iterator;

		BOOST_CONCEPT_ASSERT(( VertexListGraphConcept<InputGraph> ));
		BOOST_CONCEPT_ASSERT(( IncidenceGraphConcept<InputGraph> ));
		BOOST_CONCEPT_ASSERT(( MutableGraphConcept<InputGraph> ));

		InputGraph gout;
		std::map<InputVertex,InputVertex> mapOldNewVertices;
		auto get_vertex = [&gin,&gout,&mapOldNewVertices]
		                  (InputVertex n) -> InputVertex {
			auto it = mapOldNewVertices.find(n);
			if (it == mapOldNewVertices.cend()) {
				auto newN = add_vertex(gout);
				put(vertex_all, gout, newN, get(vertex_all, gin, n));
				it = mapOldNewVertices.emplace(n,newN).first;
			}
			return it->second;
		};

		InputVertexIterator vi,vend,vdel;
		boost::tie(vi,vend) = vertices(gin);
		for ( ; vi != vend ; ++vi) {
			if (!toKeep(*vi,gin))
				continue;

			std::stack<InputVertex,std::vector<InputVertex>> toVisit;
			toVisit.push(*vi);
			std::vector<InputVertex> visited;
			auto correspondingSrc = get_vertex(*vi);
			while (!toVisit.empty()) {
				InputVertex newV = toVisit.top();
				toVisit.pop();
				visited.push_back(newV);

				SuccessorIterator nexti,nextend;
				boost::tie(nexti,nextend) = adjacent_vertices(newV,gin);
				for ( ; nexti != nextend ; ++nexti ) {
					if (toKeep(*nexti,gin)) {
						auto correspondingDest = get_vertex(*nexti);
						add_edge(correspondingSrc,correspondingDest,gout);
					} else if (std::find(visited.cbegin(),visited.cend(),*nexti) == visited.cend()) {
						toVisit.push(*nexti);
					}
				}
			}
		}
		gin.swap(gout);
	}
}
