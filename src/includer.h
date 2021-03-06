/**
 * \file includer.h
 * \author Laurent Georget
 * \date 2015-10-23
 * \brief
 */
#include <boost/config.hpp>
#include <iostream>
#include <vector>
#include <string>

#include <boost/graph/graph_utility.hpp>
#include <boost/graph/copy.hpp>
#include <boost/property_map/property_map.hpp>

namespace boost
{
	template<typename Vertex, typename AddedGraph, typename InputGraph,
		 typename VertexIndexMap, typename BeginPredicate, typename EndPredicate>
	void expand_node(Vertex u, const AddedGraph& gadd, InputGraph& gin,
			VertexIndexMap i_map,
			BeginPredicate beginner, EndPredicate ender) {

		using InputGraphTraits = graph_traits<InputGraph>;
		using InputVertex = typename InputGraphTraits::vertex_descriptor;
		using AddedGraphTraits = graph_traits<AddedGraph>;
		using AddedVertex = typename AddedGraphTraits::vertex_descriptor;
		using size_type = typename AddedGraphTraits::vertices_size_type;

		BOOST_CONCEPT_ASSERT(( VertexListGraphConcept<AddedGraph> ));
		BOOST_CONCEPT_ASSERT(( BidirectionalGraphConcept<InputGraph> ));
		BOOST_CONCEPT_ASSERT(( MutableGraphConcept<InputGraph> ));

		std::map<AddedVertex,InputVertex> correspondence;
		auto mapOldNewVertices = make_assoc_property_map(correspondence);

		copy_graph(gadd, gin, orig_to_copy(mapOldNewVertices).
				               vertex_index_map(i_map));

		typename AddedGraphTraits::vertex_iterator v, vend;
		typename InputGraphTraits::in_edge_iterator predi, predend;
		typename InputGraphTraits::out_edge_iterator succi, succend;
		boost::tie(v, vend) = vertices(gadd);

		//special case for empty graphs
		if (v == vend) {
			for (boost::tie(predi, predend) = in_edges(u,gin); predi != predend; ++predi) {
				for (boost::tie(succi, succend) = out_edges(u,gin); succi != succend; ++succi) {
					auto maybe_edge = add_edge(source(*predi,gin),target(*succi,gin),gin);
					auto properties = get(edge_all,gin,*predi); //arbitrarily, we choose to keep the property from the in-edge
					if (maybe_edge.second)
						put(edge_all,gin,maybe_edge.first,properties);
				}
			}
		}

		// General case : non empty graph
		for (; v != vend; ++v) {
			if (beginner(*v)) { //make edges from all predecessors of u in gin to all entry nodes in gadd
				for (boost::tie(predi, predend) = in_edges(u,gin); predi != predend; ++predi) {
					auto maybe_edge = add_edge(source(*predi,gin),mapOldNewVertices[*v],gin);
					auto properties = get(edge_all,gin,*predi);
					if (maybe_edge.second)
						put(edge_all,gin,maybe_edge.first,properties);
				}
			}
			if (ender(*v)) {//make edges from all exit nodes in gadd to all successors of u in gin
				for (boost::tie(succi, succend) = out_edges(u,gin); succi != succend; ++succi) {
					auto maybe_edge = add_edge(mapOldNewVertices[*v],target(*succi,gin),gin);
					auto properties = get(edge_all,gin,*succi);
					if (maybe_edge.second)
						put(edge_all,gin,maybe_edge.first,properties);
				}
			}
		}

		clear_vertex(u,gin);
		remove_vertex(u,gin);
	}
}
