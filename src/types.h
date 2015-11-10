/**
 * \file types.h
 * \author Laurent Georget
 * \date 2015-03-03
 * \brief Useful typedefs for use with the Boost Graph Library
 */
#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

namespace kayrebt
{
	struct Node {
		unsigned int id;
		std::string label;
		std::string shape;
		std::string type;
		unsigned int line;
		std::string url;
	};
	struct Edge {
		std::string condition;
	};
	struct GraphAttr {
		std::string file;
		unsigned int line;
	};

	/**
	 * Underlying type of Boost graph used for representation
	 * and manipulation of activity diagrams
	 */
	using GraphType = boost::adjacency_list<boost::setS,boost::listS,boost::bidirectionalS,Node,Edge,GraphAttr>;
	/**
	 * Underlying type of Boost vertex descriptor for manipulation
	 * of the nodes in the activity diagrams
	 */
	using NodeDescriptor = boost::graph_traits<GraphType>::vertex_descriptor;
	/**
	 * Underlying type of Boost vertex iterator for use in Boost algorithms
	 * on the nodes in the activity diagrams
	 */
	using NodeIterator = boost::graph_traits<GraphType>::vertex_iterator;
	/**
	 * Type of Boost iterators for "inverse adjacent vertices", i.e.
	 * predecessors, of a given vertex
	 */
	using PredecessorIterator = GraphType::inv_adjacency_iterator;
	using SuccessorIterator = boost::graph_traits<GraphType>::adjacency_iterator;

	using InEdgeIterator = boost::graph_traits<GraphType>::in_edge_iterator;
	using OutEdgeIterator = boost::graph_traits<GraphType>::out_edge_iterator;

	extern const NodeDescriptor NO_NODE;
}

#endif
