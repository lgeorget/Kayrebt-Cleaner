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
}

int main(int argc, char** argv)
{
	if (argc < 3) {
		cerr << "Usage: " << argv[0] << " <graph directory> <path to main graph (relative to the graph directory)>" << endl;
		return 1;
	}

	kayrebt::GraphOutType graphin,graphout;

	dynamic_properties dp(ignore_other_properties),
	                   dp_out(ignore_other_properties),
	                   dp_add(ignore_other_properties);
	dp.property("node_id", get(&kayrebt::Node::node_id, graphin));
	dp.property("label", get(&kayrebt::Node::label, graphin));
	dp.property("shape", get(&kayrebt::Node::shape, graphin));
	dp.property("type", get(&kayrebt::Node::type, graphin));
	dp.property("label", get(&kayrebt::Edge::condition, graphin));
	dp.property("URL", get(&kayrebt::Node::url, graphin));
	dp.property("basepath", get(&kayrebt::Node::basepath, graphin));
	dp.property("mark", get(&kayrebt::Node::mark, graphin));
	dp_out.property("node_id", get(&kayrebt::Node::node_id, graphout));
	dp_out.property("label", get(&kayrebt::Node::label, graphout));
	dp_out.property("shape", get(&kayrebt::Node::shape, graphout));
	dp_out.property("type", get(&kayrebt::Node::type, graphout));
	dp_out.property("label", get(&kayrebt::Edge::condition, graphout));
	dp_out.property("URL", get(&kayrebt::Node::url, graphout));
	dp_out.property("basepath", get(&kayrebt::Node::basepath, graphout));
	dp_out.property("mark", get(&kayrebt::Node::mark, graphout));

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
		if (!read_graphviz(stream_graphin, graphin, dp, "node_id")) {
			cerr << "FATAL: Could not parse a graphviz graph from " << argv[1] << endl;
			return 2;
		}
		kayrebt::NodeOutIterator vi,vend;
		tie(vi,vend) = vertices(graphin);
		for(unsigned int index=0 ; vi != vend ; ++vi,index++) {
			graphin[*vi].node_id = index;
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

		std::vector<kayrebt::NodeOutDescriptor> vertexMap(num_vertices(graphin));
		auto inOutVertexMap = make_iterator_property_map(vertexMap.begin(),get(&kayrebt::Node::node_id,graphin));
		copy_graph(graphin,graphout,orig_to_copy(inOutVertexMap).
					    vertex_index_map(get(&kayrebt::Node::node_id,graphin)));

		kayrebt::NodeOutIterator vi,vnew,vend;
		tie(vi,vend) = vertices(graphin);
		for (vnew = vi ; vi != vend ; vi = vnew) {
			++vnew;

			auto expandable = [&graphin](const kayrebt::NodeOutDescriptor& v) {
				return !graphin[v].url.empty() &&
				       graphin[v].type == "call" &&
				       graphin[v].mark == "CALL";
			};

			auto discardable = [&graphin](const kayrebt::NodeOutDescriptor& v) {
				return graphin[v].type != "init" &&
				       graphin[v].type != "end_of_activity" &&
				       graphin[v].type != "end_of_flow" &&
				       graphin[v].mark == "DISCARDABLE";
			};

			if (discardable(*vi) || expandable(*vi)) {
				//std::cerr << "Replacing " << graphin[*vi].label << " with " << graphin[*vi].url << std::endl;
				replacements++;
				kayrebt::GraphType graphadd;
				std::string relpath, relpathbase;

				if (graphin[*vi].mark != "DISCARDABLE") {
					int posLastSlash = graphin[*vi].url.find_last_of('/');
					if (posLastSlash == 1) {
						relpathbase = graphin[*vi].basepath;
						relpath = relpathbase + graphin[*vi].url ;
					} else {
						relpathbase = graphin[*vi].url.substr(0,posLastSlash+1);
						relpath = graphin[*vi].url;
					}
					//std::cerr << "Opening " << dir << relpath << ".dot" << std::endl;
					try {
						dp_add.property("node_id", get(&kayrebt::Node::node_id, graphadd));
						dp_add.property("label", get(&kayrebt::Node::label, graphadd));
						dp_add.property("shape", get(&kayrebt::Node::shape, graphadd));
						dp_add.property("type", get(&kayrebt::Node::type, graphadd));
						dp_add.property("label", get(&kayrebt::Edge::condition, graphadd));
						dp_add.property("URL", get(&kayrebt::Node::url, graphadd));
						dp_add.property("basepath", get(&kayrebt::Node::basepath, graphadd));
						dp_add.property("mark", get(&kayrebt::Node::mark, graphadd));
						std::ifstream stream_graphadd(dir + relpath + ".dot");
						if (!read_graphviz(stream_graphadd,graphadd,  dp_add, "node_id")) {
							//cerr << "ERROR: Could not parse a graphviz graph from " << argv[1] << endl;
							continue; //if something went wrong, we just leave the node as it is
						}
					} catch (graph_exception& e) {
						//cerr << "ERROR: The graph could not be parsed: " << e.what() << endl;
						graphout[inOutVertexMap[*vi]].type = "non-expandable call";
						continue;
					}
				}
				auto filtered_gadd = filter_graph(graphadd, EdgePredicate(&graphadd), NodePredicate(&graphadd));

				auto beginPredicate = [&graphadd](kayrebt::NodeDescriptor n) {
					kayrebt::PredecessorIterator vi,vend;
					boost::tie(vi,vend) = inv_adjacent_vertices(n,graphadd);
					return std::any_of(vi, vend,
						[&graphadd](const kayrebt::NodeDescriptor& pred) {
							return graphadd[pred].type == "init";
						});
				};
				auto endPredicate = [&graphadd](kayrebt::NodeDescriptor n) {
					kayrebt::SuccessorIterator vi,vend;
					boost::tie(vi,vend) = adjacent_vertices(n,graphadd);
					return std::any_of(vi, vend,
						[&graphadd](const kayrebt::NodeDescriptor& succ) {
							return graphadd[succ].type == "end_of_activity";
						});
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
		std::cerr << "Finished round " << round++ << " ; " << num_vertices(graphout) << " vertices" << std::endl;

		//Ensure that each node has a unique index in the range [0;num_vertices(graphout)-1]
		kayrebt::NodeOutIterator vouti,voutend;
		tie(vouti,voutend) = vertices(graphout);
		for (unsigned int index=0 ; vouti != voutend ; index++, ++vouti)
			graphout[*vouti].node_id = index;

		graphin.clear();
		graphin.swap(graphout);
	} while (replacements > 0 && tries > 0);

	write_graphviz_dp(std::cout,graphin,dp/*,"node_id"*/);
	return 0;
}
