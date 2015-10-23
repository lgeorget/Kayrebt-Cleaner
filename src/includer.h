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
#include <boost/graph/filtered_graph.hpp>
#include <boost/property_map/property_map.hpp>

namespace boost
{
	template <typename NodeTypeMap>
	struct NotInitNorEndNode {
		NotInitNorEndNode() {}
		NotInitNorEndNode(const NodeTypeMap* m) : _typeMap(m) {}
		template<typename Vertex>
		bool operator()(const Vertex& v) const {
			return (*_typeMap)[v] != "INIT" && (*_typeMap)[v] != "END_OF_ACTIVITY";
		}
		const NodeTypeMap* _typeMap;
	};
	template <typename NodeTypeMap,typename Graph>
	struct NotInitNorEndEdge {
		NotInitNorEndEdge() {}
		NotInitNorEndEdge(const NodeTypeMap* m, const Graph* g) : _typeMap(m), _g(g) {}
		template<typename Edge>
		bool operator()(const Edge& e) const {
			return (*_typeMap)[source(e,_g)] != "INIT" && (*_typeMap)[target(e,_g)] != "END_OF_ACTIVITY";
		}
		const NodeTypeMap* _typeMap;
		const Graph* _g;
	};

	template<typename Vertex, typename AddedGraph, typename InputGraph,
		 typename NodeTypeMap, typename VertexIndexMap>
	void expand_node(Vertex u, AddedGraph gadd, InputGraph gin,
			NodeTypeMap nodeType, VertexIndexMap i_map) {

		using InputGraphTraits = graph_traits<InputGraph>;
		using InputVertex = typename InputGraphTraits::vertex_descriptor;
		using AddedGraphTraits = graph_traits<AddedGraph>;
		using AddedVertex = typename AddedGraphTraits::vertex_descriptor;
		using size_type = typename AddedGraphTraits::vertices_size_type;

		BOOST_CONCEPT_ASSERT(( VertexListGraphConcept<AddedGraph> ));
		BOOST_CONCEPT_ASSERT(( BidirectionalGraphConcept<InputGraph> ));
		BOOST_CONCEPT_ASSERT(( MutableGraphConcept<InputGraph> ));
		BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<NodeTypeMap,AddedVertex> ));
		BOOST_STATIC_ASSERT(( is_same<typename property_traits<NodeTypeMap>::value_type,std::string>::value ));

		std::vector<InputVertex> correspondence(num_vertices(gadd));
		auto mapOldNewVertices = make_iterator_property_map(
			correspondence.begin(),get(vertex_index,gadd));

		using EdgePredicate = NotInitNorEndEdge<NodeTypeMap,AddedGraph>;
		using NodePredicate = NotInitNorEndNode<NodeTypeMap>;
		using FilteredAddedGraph = filtered_graph<AddedGraph,EdgePredicate,NodePredicate>;
		FilteredAddedGraph filtered_gadd(gadd,EdgePredicate(&nodeType,&gadd),NodePredicate(&nodeType));

		copy_graph(filtered_gadd, gin, orig_to_copy(mapOldNewVertices).
				               vertex_index_map(i_map));

		typename AddedGraphTraits::vertex_iterator v, vend;
		typename AddedGraphTraits::adjacency_iterator ai, aend;
		typename InputGraphTraits::inv_adjacency_iterator predi, predend;
		typename InputGraphTraits::adjacency_iterator succi, succend;
		for (boost::tie(v, vend) = vertices(gadd); v != vend; ++v) {
			if (nodeType[*v] == "INIT") { //make edges from all predecessors of u in gin to all successors of INIT in gadd
				for (boost::tie(predi, predend) = inv_adjacent_vertices(u,gin); predi != predend; ++predi)
					for (boost::tie(ai, aend) = adjacent_vertices(*v,gadd); ai != aend; ++ai)
						add_edge(*predi,mapOldNewVertices[*ai],gin);
			} else {
				for (boost::tie(ai, aend) = adjacent_vertices(*v,gadd); ai != aend; ++ai)
					if (nodeType[*v] == "END_OF_ACTIVITY") //make edges from all predecessors of END in gadd to all successors of u in gin
						for (boost::tie(succi, succend) = adjacent_vertices(u,gin); succi != succend; ++succi)
							add_edge(mapOldNewVertices[*v],*succi);
			}
		}

		clear_vertex(u,gin);
		remove_vertex(u,gin);
	}
}
