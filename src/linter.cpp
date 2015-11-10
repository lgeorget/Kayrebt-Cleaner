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
#include <boost/property_map/transform_value_property_map.hpp>
#include "linter.h"

using namespace std;
using namespace boost;

namespace kayrebt
{
	struct Node {
		unsigned int id;
		unsigned int node_id;
		std::string label;
		std::string shape;
		std::string type;
		unsigned int line;
		std::string url;
		std::string basepath;
		std::string mark;

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
	using GraphType = boost::adjacency_list<boost::vecS,boost::listS,boost::bidirectionalS,Node,Edge,GraphAttr>;
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
	using SuccessorIterator = boost::adjacency_iterator_generator<GraphType>::type;
	/**
	 * Type of Boost iterators for "inverse adjacent vertices", i.e.
	 * predecessors, of a given vertex
	 */
	using PredecessorIterator = boost::inv_adjacency_iterator_generator<GraphType,NodeDescriptor,boost::graph_traits<GraphType>::in_edge_iterator>::type;
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
	class NodePredicate {
	public:
		NodePredicate() {}
		bool operator()(kayrebt::NodeDescriptor n, const kayrebt::GraphType& g) const {
			return g[n].type == "init" ||
			       g[n].type == "end_of_activity" ||
			       g[n].type == "end_of_flow" ||
			       g[n].mark != "DISCARDABLE";
		}
	};
}

int main(int argc, char** argv)
{
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " <graph>" << endl;
		return 1;
	}

	kayrebt::GraphType graphin;

	dynamic_properties dp(ignore_other_properties);
	dp.property("node_id", get(&kayrebt::Node::node_id, graphin));
	dp.property("label", get(&kayrebt::Node::label, graphin));
	dp.property("shape", get(&kayrebt::Node::shape, graphin));
	dp.property("type", get(&kayrebt::Node::type, graphin));
	dp.property("label", get(&kayrebt::Edge::condition, graphin));
	dp.property("URL", get(&kayrebt::Node::url, graphin));
	dp.property("basepath", get(&kayrebt::Node::basepath, graphin));
	dp.property("mark", get(&kayrebt::Node::mark, graphin));

	std::string inputGraph(argv[1]);
	if (inputGraph.find(".dot",inputGraph.length() - 4) == std::string::npos)
		inputGraph += ".dot";

	try {
		std::ifstream stream_graphin(inputGraph);
		if (!read_graphviz(stream_graphin, graphin, dp)) {
			cerr << "FATAL: Could not parse a graphviz graph from " << argv[1] << endl;
			return 2;
		}
	} catch (graph_exception& e) {
		cerr << "FATAL: The graph could not be parsed: " << e.what() << endl;
	        return 2;
	}

	kayrebt::NodeIterator vi,vend, init;
	boost::tie(vi,vend) = vertices(graphin);
	for (init = vend ; vi != vend && init == vend ; ++vi)
		if (graphin[*vi].type == "init")
			init = vi;

	if (init == vend) {
		std::cerr << "No initial node" << std::endl;
		return 4;
	}

	boost::tie(vi,vend) = vertices(graphin);
	for (unsigned int index=0 ; vi != vend ; index++, ++vi)
		graphin[*vi].id = index;

	lint(graphin, NodePredicate());
	std::cerr << "Linted!" << std::endl;
	write_graphviz_dp(std::cout,graphin,dp);
}
