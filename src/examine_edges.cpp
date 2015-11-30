#include <boost/config.hpp>
#include <iostream>
#include <vector>
#include <stack>
#include <tuple>
#include <algorithm>
#include <locale>
#include <iterator>

#include <boost/graph/graphviz.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/graph/graph_utility.hpp>
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
		std::string mark;
		std::vector<unsigned int> predecessors;
		std::vector<std::string> args;
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
	using PredecessorIterator = GraphType::inv_adjacency_iterator;
	using SuccessorIterator = boost::graph_traits<GraphType>::adjacency_iterator;

	using InEdgeIterator = boost::graph_traits<GraphType>::in_edge_iterator;
	using OutEdgeIterator = boost::graph_traits<GraphType>::out_edge_iterator;

	extern const NodeDescriptor NO_NODE;

	struct comma_is_separator : std::ctype<char> {
		comma_is_separator(std::size_t refs = 0) : std::ctype<char>(get_table(), false, refs) {}
		static mask const* get_table()
		{
			static std::vector<mask> v{classic_table(), classic_table() + table_size};
			v[',']  |= space;
			v['"']  |= space;
			v['\n'] &= ~space;
			v[' '] &= ~space;
			return &v[0];
		}
	};
}

// It's boost-compliant to add explicit template specialization to namespace
namespace boost {
namespace conversion {
namespace detail {
	template<>
	bool try_lexical_convert<std::vector<unsigned int>,std::string>(const std::string& arg, std::vector<unsigned int>& result)
	{
		std::istringstream it(arg);
		it.imbue(std::locale(it.getloc(), new kayrebt::comma_is_separator));
		result = std::vector<unsigned int>{
				std::istream_iterator<unsigned int>(it),
				std::istream_iterator<unsigned int>()
		};

		return it.eof();
	}

	template<>
	bool try_lexical_convert<std::vector<std::string>,std::string>(const std::string& arg, std::vector<std::string>& result)
	{
		std::istringstream it(arg);
		it.imbue(std::locale(it.getloc(), new kayrebt::comma_is_separator));
		result = std::vector<std::string>{
				std::istream_iterator<std::string>(it),
				std::istream_iterator<std::string>()
		};

		return it.eof();
	}
}
}
}

namespace std {
std::ostream& operator<<(std::ostream& out, const std::vector<unsigned int>& arg)
{
	std::copy(arg.cbegin(), arg.cend(), std::ostream_iterator<unsigned int>(out,","));
	return out;
}
std::ostream& operator<<(std::ostream& out, const std::vector<std::string>& arg)
{
	std::copy(arg.cbegin(), arg.cend(), std::ostream_iterator<std::string>(out,","));
	return out;
}
}

using namespace boost;
using namespace kayrebt;

using State = std::tuple<NodeDescriptor,
                         std::vector<NodeDescriptor>,
                         std::vector<std::string>,
                         int>;


inline EdgeDescriptor pred(NodeDescriptor n, GraphType& graph)
{
	assert(in_degree(n,graph) == 1);
	kayrebt::InEdgeIterator predi;
	std::tie(predi,std::ignore) = in_edges(n,graph);
	return *predi;
}

void examine_all_paths(NodeDescriptor nf, NodeDescriptor no, GraphType& graph)
{
	std::stack<State> states;
	states.push(State{nf, {}, {}, -1});
	while (!states.empty()) {
		NodeDescriptor n;
		std::vector<NodeDescriptor> l;
		std::vector<std::string> c;
		int w = -1;
		std::tie(n,l,c,w) = states.top();
		states.pop();

		std::cerr << "Reached node " << graph[n].id << "\n";
		std::cerr << "Path: ";
		std::transform(l.cbegin(), l.cend(),
				std::ostream_iterator<unsigned int>(std::cerr, " - "),
				[&graph](const NodeDescriptor& n) { return graph[n].id; });
		std::cerr << std::endl;

		if (n == no) //base case, we reached the observation node, OK
			continue;

		if (std::find(l.cbegin(), l.cend(), n) != l.cend()) //we found a loop
			continue;

		if (graph[n].type == "assign") {
			EdgeDescriptor p = pred(n, graph);
			c.push_back(graph[n].label);
			if (!graph[p].condition.empty())
				c.push_back(graph[p].condition);

			l.push_back(n);
			states.push(State{source(p,graph), l, c, w});
		} else if (graph[n].type == "phi") {
			std::cerr << "number of args " << graph[n].args.size() << std::endl;
			for (unsigned int i = 0 ; i < graph[n].args.size() ; i++) {
				std::vector<std::string> newc{c};
				newc.push_back(graph[n].label.substr(0,graph[n].label.find(" = ")) + " = " + graph[n].args[i]);
				EdgeDescriptor p = pred(n, graph);
				if (!graph[p].condition.empty())
					newc.push_back(graph[p].condition);
				std::vector<NodeDescriptor> newl{l};
				newl.push_back(n);
				states.push(State{source(p,graph), std::move(newl), std::move(newc), graph[n].predecessors[i]});
			}
		} else if (in_degree(n, graph) > 0) {
			kayrebt::InEdgeIterator predi, predend;
			for (std::tie(predi,predend) = in_edges(n, graph) ;
			     predi != predend ;
			     ++predi) {
				NodeDescriptor maybe_pred = source(*predi,graph);
				if (w == -1 || graph[maybe_pred].id == static_cast<unsigned int>(w)) {
					std::vector<NodeDescriptor> newl{l};
					newl.push_back(n);
					std::vector<std::string> newc{c};
					newc.push_back(graph[*predi].condition);
					states.push(State{source(*predi,graph), std::move(newl), c, -1});
				}
			}
		} else {
			std::cout << "possible path ";
			std::transform(l.cbegin(), l.cend(),
					std::ostream_iterator<std::string>(std::cout, "\n"),
					[&graph](kayrebt::NodeDescriptor n) {
						return std::to_string(graph[n].id) +
						" (" + graph[n].label + ")";
					});
			std::cout << std::endl;
			std::cout << "Associated constraints: ";
			std::copy(c.cbegin(), c.cend(),
					std::ostream_iterator<std::string>(std::cout, " && "));
		}

	}
}


int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <graph>" << std::endl;
		return 1;
	}

	GraphType graphin;

	dynamic_properties dp{ignore_other_properties};
	dp.property("node_id", get(&kayrebt::Node::id, graphin));
	dp.property("label", get(&kayrebt::Node::label, graphin));
	dp.property("shape", get(&kayrebt::Node::shape, graphin));
	dp.property("type", get(&kayrebt::Node::type, graphin));
	dp.property("label", get(&kayrebt::Edge::condition, graphin));
	dp.property("URL", get(&kayrebt::Node::url, graphin));
	dp.property("mark", get(&kayrebt::Node::mark, graphin));
	dp.property("predecessors", get(&kayrebt::Node::predecessors, graphin));
	dp.property("args", get(&kayrebt::Node::args, graphin));

	std::string filename(argv[1]);
	std::ifstream file(filename);
	if (!file) {
		throw std::runtime_error("File not found");
	}
	if (!read_graphviz(file, graphin, dp, "node_id")) {
		throw std::runtime_error(std::string("Could not parse a graphviz graph from ") + argv[1]);
	}

	std::vector<NodeDescriptor> nflows, nobs;

	kayrebt::NodeIterator vi, vend;
	boost::tie(vi,vend) = vertices(graphin);
	std::copy_if(vi, vend, std::back_inserter(nflows),
			[&graphin](const NodeDescriptor& n) {
				return graphin[n].mark == "FLOW";
			});
	boost::tie(vi,vend) = vertices(graphin);
	std::copy_if(vi, vend, std::back_inserter(nobs),
			[&graphin](const NodeDescriptor& n) {
				return graphin[n].mark == "LSM";
			});

	for (auto&& nf : nflows)
		for (auto&& no : nobs)
			examine_all_paths(nf, no, graphin);
}
