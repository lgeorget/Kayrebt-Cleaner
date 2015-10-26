/**
 * \file includer.cpp
 * \author Laurent Georget
 * \date 2015-10-23
 * \brief
 */
#include <iostream>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/copy.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <cstring>
#include <bcg_user.h>
#include "includer.h"

using namespace std;
using namespace boost;

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
	using GraphType = boost::adjacency_list<boost::setS,boost::vecS,boost::bidirectionalS,Node,Edge,GraphAttr>;
	/**
	 * Underlying type of Boost vertex descriptor for manipulation
	 * of the nodes in the activity diagrams
	 */
	using NodeDescriptor = boost::graph_traits<GraphType>::vertex_descriptor;
	using EdgeDescriptor = boost::graph_traits<GraphType>::edge_descriptor;
	/**
	 * Underlying type of Boost vertex iterator for use in Boost algorithms
	 * on the nodes in the activity diagrams
	 */
	using NodeIterator = boost::graph_traits<GraphType>::vertex_iterator;
	/**
	 * Type of Boost iterators for "inverse adjacent vertices", i.e.
	 * predecessors, of a given vertex
	 */
	using PredecessorIterator = boost::inv_adjacency_iterator_generator<GraphType,NodeDescriptor,boost::graph_traits<GraphType>::in_edge_iterator>::type;

	template<typename Graph, typename EdgePredicate,typename VertexPredicate>
	filtered_graph<Graph,EdgePredicate,VertexPredicate>
	filter_graph(Graph& g, EdgePredicate ep, VertexPredicate vp) {
		return filtered_graph<Graph,EdgePredicate,VertexPredicate>(g,ep,vp);
	}
}

/**
 * \brief 
 *
 * \param argc
 * \param argv
 *
 * \return 
 */

namespace {
	class EdgePredicate {
	public:
		EdgePredicate() {}
		EdgePredicate(const kayrebt::GraphType* graph) : _graph(graph) {}
		bool operator()(kayrebt::EdgeDescriptor e) const {
			return (*_graph)[source(e,*_graph)].type != "init" &&
			       (*_graph)[target(e,*_graph)].type != "end_of_activity";
		}

	private:
		const kayrebt::GraphType* _graph;
	};

	class NodePredicate {
	public:
		NodePredicate() {}
		NodePredicate(const kayrebt::GraphType* graph) : _graph(graph) {}
		bool operator()(kayrebt::NodeDescriptor n) const {
			return (*_graph)[n].type != "init" &&
			       (*_graph)[n].type != "end_of_activity";
		}

	private:
		const kayrebt::GraphType* _graph;
	};
}

int main(int argc, char** argv)
{
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " <graph.dot> <graph_to_add.dot>" << endl;
		return 1;
	}

	kayrebt::GraphType graphin, graphadd;

	dynamic_properties dp_in(ignore_other_properties);
	dynamic_properties dp_add(ignore_other_properties);
	dp_in.property("node_id", get(&kayrebt::Node::id, graphin));
	dp_in.property("label", get(&kayrebt::Node::label, graphin));
	dp_in.property("shape", get(&kayrebt::Node::shape, graphin));
	dp_in.property("label", get(&kayrebt::Edge::condition, graphin));
	dp_add.property("node_id", get(&kayrebt::Node::id, graphadd));
	dp_add.property("label", get(&kayrebt::Node::label, graphadd));
	dp_add.property("shape", get(&kayrebt::Node::shape, graphadd));
	dp_add.property("type", get(&kayrebt::Node::type, graphadd));
	dp_add.property("label", get(&kayrebt::Edge::condition, graphadd));

	std::ifstream stream_graphin(argv[1]);
	std::ifstream stream_graphadd(argv[2]);

	try {
		if (!read_graphviz(stream_graphin, graphin, dp_in)) {
			cerr << "FATAL: Could not parse a graphviz graph from " << argv[1] << endl;
			return 2;
		}
		stream_graphin.close();
		if (!read_graphviz(stream_graphadd,graphadd,  dp_add)) {
			cerr << "FATAL: Could not parse a graphviz graph from " << argv[1] << endl;
			return 2;
		}
		stream_graphadd.close();
	} catch (graph_exception& e) {
		cerr << "FATAL: The graph could not be parsed: " << e.what() << endl;
	        return 2;
	}

	auto filtered_gadd = filter_graph(graphadd, EdgePredicate(&graphadd), NodePredicate(&graphadd));

	auto beginPredicate = [&filtered_gadd](kayrebt::NodeDescriptor n) {
		return in_degree(n,filtered_gadd) == 0;
	};
	auto endPredicate = [&filtered_gadd](kayrebt::NodeDescriptor n) {
		return out_degree(n,filtered_gadd) == 0;
	};

	expand_node(3,filtered_gadd,graphin,get(vertex_index,filtered_gadd),beginPredicate,endPredicate);
	write_graphviz_dp(std::cout,graphin,dp_in);
	return 0;
}
