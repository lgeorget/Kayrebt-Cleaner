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
#include <cstring>
#include <bcg_user.h>
#include "includer.h"

using namespace std;
using namespace boost;

namespace kayrebt
{
	struct Node {
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
	using GraphType = boost::adjacency_list<boost::vecS,boost::vecS,boost::bidirectionalS,Node,Edge,GraphAttr>;
	using GraphOutType = boost::adjacency_list<boost::setS,boost::listS,boost::bidirectionalS,Node,Edge,GraphAttr>;
	/**
	 * Underlying type of Boost vertex descriptor for manipulation
	 * of the nodes in the activity diagrams
	 */
	using NodeDescriptor = boost::graph_traits<GraphType>::vertex_descriptor;
	using NodeOutDescriptor = boost::graph_traits<GraphOutType>::vertex_descriptor;
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
			       (*_graph)[target(e,*_graph)].type != "end_of_activity" &&
			       (*_graph)[target(e,*_graph)].type != "end_of_flow";
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
			       (*_graph)[n].type != "end_of_activity" &&
			       (*_graph)[n].type != "end_of_flow";
		}

	private:
		const kayrebt::GraphType* _graph;
	};

	class TransformIndex {
	public:
		TransformIndex(std::string prefix) : _prefix(prefix) {}
		std::string operator()(const std::string& index) const {
			return _prefix + index;
		}

	private:
		std::string _prefix;
	};
}

int main(int argc, char** argv)
{
	if (argc < 3) {
		cerr << "Usage: " << argv[0] << " <graph.dot> <path to included graphs>" << endl;
		return 1;
	}

	kayrebt::GraphType graphin;
	kayrebt::GraphOutType graphout;

	dynamic_properties dp_in(ignore_other_properties);
	dp_in.property("label", get(&kayrebt::Node::label, graphin));
	dp_in.property("shape", get(&kayrebt::Node::shape, graphin));
	dp_in.property("type", get(&kayrebt::Node::type, graphin));
	dp_in.property("label", get(&kayrebt::Edge::condition, graphin));
	dp_in.property("URL", get(&kayrebt::Node::url, graphin));
	dynamic_properties dp_out(ignore_other_properties);
	dp_out.property("label", get(&kayrebt::Node::label, graphout));
	dp_out.property("shape", get(&kayrebt::Node::shape, graphout));
	dp_out.property("type", get(&kayrebt::Node::type, graphout));
	dp_out.property("label", get(&kayrebt::Edge::condition, graphout));
	dp_out.property("URL", get(&kayrebt::Node::url, graphout));

	try {
		std::ifstream stream_graphin(argv[1]);
		if (!read_graphviz(stream_graphin, graphin, dp_in, "id")) {
			cerr << "FATAL: Could not parse a graphviz graph from " << argv[1] << endl;
			return 2;
		}
		stream_graphin.close();
	} catch (graph_exception& e) {
		cerr << "FATAL: The graph could not be parsed: " << e.what() << endl;
	        return 2;
	}

	std::map<kayrebt::NodeDescriptor,kayrebt::NodeOutDescriptor> vertexMap;

	auto inOutVertexMap = make_assoc_property_map(vertexMap);
	copy_graph(graphin,graphout,orig_to_copy(inOutVertexMap));

	kayrebt::NodeIterator vi,vnew,vend;
	tie(vi,vend) = vertices(graphin);
	for (vnew = vi ; vi != vend ; vi = vnew) {
		++vnew;
		if (graphin[*vi].type == "call" && !graphin[*vi].url.empty()) {
			std::cerr << "Replacing " << graphin[*vi].label << " with " << graphin[*vi].url << std::endl;
			std::string path;
			if (graphin[*vi].url.find_last_of('/') == 1)
				path = "fs/open.c/" + graphin[*vi].url;
			else
				path = graphin[*vi].url;
			std::cerr << "Opening " << argv[2] << path << ".dot" << std::endl;
			kayrebt::GraphType graphadd;
			dynamic_properties dp_add(ignore_other_properties);
			dp_add.property("label", get(&kayrebt::Node::label, graphadd));
			dp_add.property("shape", get(&kayrebt::Node::shape, graphadd));
			dp_add.property("type", get(&kayrebt::Node::type, graphadd));
			dp_add.property("label", get(&kayrebt::Edge::condition, graphadd));
			dp_add.property("URL", get(&kayrebt::Node::url, graphadd));
			try {
				std::ifstream stream_graphadd(argv[2] + path + ".dot");
				if (!read_graphviz(stream_graphadd,graphadd,  dp_add, "id")) {
					cerr << "ERROR: Could not parse a graphviz graph from " << argv[1] << endl;
					continue; //if something went wrong, we just leave the node as it is
				}
			} catch (graph_exception& e) {
				cerr << "ERROR: The graph could not be parsed: " << e.what() << endl;
				continue;
			}
			auto filtered_gadd = filter_graph(graphadd, EdgePredicate(&graphadd), NodePredicate(&graphadd));

			auto beginPredicate = [&filtered_gadd](kayrebt::NodeDescriptor n) {
				return in_degree(n,filtered_gadd) == 0;
			};
			auto endPredicate = [&filtered_gadd,&graphadd](kayrebt::NodeDescriptor n) {
				if (out_degree(n,filtered_gadd) > 0)
					return false;
				bool result = true;
				kayrebt::SuccessorIterator vi,vend;
				for (boost::tie(vi,vend) = adjacent_vertices(n,graphadd) ; vi != vend && result ; ++vi) {
					result = graphadd[*vi].type == "end_of_activity";
				}
				return result;
			};

			//We have to remap the node because filtering could
			//have introduced 'holes' in the vertex_index map
			std::map<kayrebt::NodeDescriptor,unsigned int> vertexAddIndexMap;
			graph_traits<decltype(filtered_gadd)>::vertex_iterator vaddi,vaddend;
			tie(vaddi,vaddend) = vertices(filtered_gadd);
			for (unsigned int index=0 ; vaddi != vaddend ; index++, ++vaddi)
				vertexAddIndexMap.emplace(*vaddi,index);
			expand_node(inOutVertexMap[*vi],filtered_gadd,graphout,
				    make_assoc_property_map(vertexAddIndexMap),
				    beginPredicate,endPredicate);
		}
	}

	//Ensure that each node has a unique index
	std::map<kayrebt::NodeOutDescriptor,unsigned int> vertexOutIndexMap;
	graph_traits<decltype(graphout)>::vertex_iterator vouti,voutend;
	tie(vouti,voutend) = vertices(graphout);
	for (unsigned int index=0 ; vouti != voutend ; index++, ++vouti)
		vertexOutIndexMap.emplace(*vouti,index);
	dp_out.property("node_id",make_assoc_property_map(vertexOutIndexMap));
	write_graphviz_dp(std::cout,graphout,dp_out/*,"node_id"*/);
	return 0;
}
