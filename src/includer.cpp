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
		std::string basepath;

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
	using NodeOutIterator = boost::graph_traits<GraphOutType>::vertex_iterator;
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
		cerr << "Usage: " << argv[0] << " <graph directory> <path to main graph (relative to the graph directory)>" << endl;
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
	dp_in.property("basepath", get(&kayrebt::Node::basepath, graphin));
	dynamic_properties dp_out(ignore_other_properties);
	dp_out.property("label", get(&kayrebt::Node::label, graphout));
	dp_out.property("shape", get(&kayrebt::Node::shape, graphout));
	dp_out.property("type", get(&kayrebt::Node::type, graphout));
	dp_out.property("label", get(&kayrebt::Edge::condition, graphout));
	dp_out.property("URL", get(&kayrebt::Node::url, graphout));
	dp_out.property("basepath", get(&kayrebt::Node::basepath, graphout));

	std::string dir(argv[1]);
	if (dir[dir.length() - 1] != '/')
		dir += "/";
	std::string inputGraph(argv[2]);
	if (inputGraph.find(".dot",inputGraph.length() - 4) == std::string::npos)
		inputGraph += ".dot";
	std::string basepath(inputGraph);
	basepath.erase(basepath.find_last_of('/') + 1);

	try {
		std::ifstream stream_graphin(dir + inputGraph);
		if (!read_graphviz(stream_graphin, graphin, dp_in, "id")) {
			cerr << "FATAL: Could not parse a graphviz graph from " << argv[1] << endl;
			return 2;
		}
		kayrebt::NodeIterator vi,vend;
		for(tie(vi,vend) = vertices(graphin) ; vi != vend ; ++vi) {
			if (graphin[*vi].type == "call")
				graphin[*vi].basepath = basepath;
		}
	} catch (graph_exception& e) {
		cerr << "FATAL: The graph could not be parsed: " << e.what() << endl;
	        return 2;
	}

	int replacements;
	int tries = argc >= 4 ? std::atoi(argv[3]) : 5;
	std::map<kayrebt::NodeOutDescriptor,unsigned int> vertexOutIndexMap; //we need that at the end
	do {
		replacements = 0;
		tries--;
		graphout.clear();

		std::vector<kayrebt::NodeOutDescriptor> vertexMap(num_vertices(graphin));
		auto inOutVertexMap = make_iterator_property_map(vertexMap.begin(),get(vertex_index,graphin));
		copy_graph(graphin,graphout,orig_to_copy(inOutVertexMap));

		kayrebt::NodeIterator vi,vnew,vend;
		tie(vi,vend) = vertices(graphin);
		for (vnew = vi ; vi != vend ; vi = vnew) {
			++vnew;

			auto expandable = [&graphin](const kayrebt::NodeDescriptor& v) {
				return graphin[v].label.find("__builtin") == std::string::npos &&
					graphin[v].label.find("ERR") == std::string::npos &&
					graphin[v].label.find("current") == std::string::npos &&
					graphin[v].label.find("lock") == std::string::npos &&
					graphin[v].label.find("lru") == std::string::npos &&
					graphin[v].label.find("rcu") == std::string::npos &&
					graphin[v].label.find("arch") == std::string::npos &&
					graphin[v].label.find("native") == std::string::npos &&
					graphin[v].label.find("__read_once_size") == std::string::npos &&
					graphin[v].label.find("atomic") == std::string::npos &&
					graphin[v].label.find("log") == std::string::npos &&
					graphin[v].label.find("print") == std::string::npos &&
					graphin[v].label.find("bit") == std::string::npos &&
					graphin[v].label.find("cpu") == std::string::npos &&
					graphin[v].url.find("mm/") == std::string::npos &&
					graphin[v].url.find("arch/") == std::string::npos &&
					graphin[v].url.find("panic.c/") == std::string::npos &&
					!graphin[v].url.empty();
			};

			if (graphin[*vi].type == "call" && expandable(*vi)) {
				std::cerr << "Replacing " << graphin[*vi].label << " with " << graphin[*vi].url << std::endl;
				replacements++;
				std::string relpath, relpathbase;
				int posLastSlash = graphin[*vi].url.find_last_of('/');
				if (posLastSlash == 1) {
					relpathbase = graphin[*vi].basepath;
					relpath = relpathbase + graphin[*vi].url ;
				} else {
					relpathbase = graphin[*vi].url.substr(0,posLastSlash+1);
					relpath = graphin[*vi].url;
				}
				std::cerr << "Opening " << dir << relpath << ".dot" << std::endl;
				kayrebt::GraphType graphadd;
				dynamic_properties dp_add(ignore_other_properties);
				dp_add.property("label", get(&kayrebt::Node::label, graphadd));
				dp_add.property("shape", get(&kayrebt::Node::shape, graphadd));
				dp_add.property("type", get(&kayrebt::Node::type, graphadd));
				dp_add.property("label", get(&kayrebt::Edge::condition, graphadd));
				dp_add.property("URL", get(&kayrebt::Node::url, graphadd));
				dp_add.property("basepath", get(&kayrebt::Node::basepath, graphadd));
				try {
					std::ifstream stream_graphadd(dir + relpath + ".dot");
					if (!read_graphviz(stream_graphadd,graphadd,  dp_add, "id")) {
						cerr << "ERROR: Could not parse a graphviz graph from " << argv[1] << endl;
						continue; //if something went wrong, we just leave the node as it is
					}
				} catch (graph_exception& e) {
					cerr << "ERROR: The graph could not be parsed: " << e.what() << endl;
					graphout[inOutVertexMap[*vi]].type = "non-expandable call";
					continue;
				}
				auto filtered_gadd = filter_graph(graphadd, EdgePredicate(&graphadd), NodePredicate(&graphadd));

				auto beginPredicate = [&graphadd](kayrebt::NodeDescriptor n) {
					bool result = false;
					kayrebt::PredecessorIterator vi,vend;
					for (boost::tie(vi,vend) = inv_adjacent_vertices(n,graphadd) ; vi != vend && !result ; ++vi) {
						result = graphadd[*vi].type == "init";
					}
					return result;
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

				//We have to remap the nodes because filtering could
				//have introduced 'holes' in the vertex_index map
				//
				//Here, we can also modify the attribute of graphadd
				//before expansion
				std::map<kayrebt::NodeDescriptor,unsigned int> vertexAddIndexMap;
				graph_traits<decltype(filtered_gadd)>::vertex_iterator vaddi,vaddend;
				tie(vaddi,vaddend) = vertices(filtered_gadd);
				for (unsigned int index=0 ; vaddi != vaddend ; index++, ++vaddi) {
					vertexAddIndexMap.emplace(*vaddi,index);
					if (filtered_gadd[*vaddi].type == "call")
						filtered_gadd[*vaddi].basepath = relpathbase;
				}
				expand_node(inOutVertexMap[*vi],filtered_gadd,graphout,
						make_assoc_property_map(vertexAddIndexMap),
						beginPredicate,endPredicate);
			}
		}
		static int round = 1;
		std::cerr << "Finished round " << round++ << std::endl;

		//Ensure that each node has a unique index
		vertexOutIndexMap.clear();
		kayrebt::NodeOutIterator vouti,voutend;
		tie(vouti,voutend) = vertices(graphout);
		for (unsigned int index=0 ; vouti != voutend ; index++, ++vouti)
			vertexOutIndexMap.emplace(*vouti,index);
		dp_out.property("node_id",make_assoc_property_map(vertexOutIndexMap));

		graphin.clear();
		copy_graph(graphout, graphin, vertex_index_map(make_assoc_property_map(vertexOutIndexMap)));
	} while (replacements > 0 && tries > 0);

	write_graphviz_dp(std::cout,graphout,dp_out/*,"node_id"*/);
	return 0;
}
