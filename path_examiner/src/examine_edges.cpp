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

#include "converters.h"
#include "types.h"
#include "constraint_parser.h"

using namespace boost;
using namespace kayrebt;

using State = std::tuple<NodeDescriptor,
                         std::vector<NodeDescriptor>,
                         std::vector<Constraint>,
                         int>;


inline EdgeDescriptor pred(NodeDescriptor n, GraphType& graph)
{
	kayrebt::InEdgeIterator predi;
	std::tie(predi,std::ignore) = in_edges(n,graph);
	return *predi;
}

void examine_all_paths(NodeDescriptor nf, NodeDescriptor no, GraphType& graph)
{
	std::stack<State> states;

	states.push(State{nf, {}, {}, -1});
	while (!states.empty()) {
		auto topstate = states.top();
		NodeDescriptor n = std::get<0>(topstate);
		std::vector<NodeDescriptor>& l = std::get<1>(topstate);
		std::vector<Constraint>& c = std::get<2>(topstate);
		int w = std::get<3>(topstate);
		states.pop();

#ifndef NDEBUG
		std::cerr << "Reached node " << graph[n].id << "\n";
		std::cerr << "Path: ";
		std::transform(l.cbegin(), l.cend(),
				std::ostream_iterator<unsigned int>(std::cerr, " - "),
				[&graph](const NodeDescriptor& n) { return graph[n].id; });
		std::cerr << std::endl;
#endif

		if (n == no) //base case, we reached the observation node, OK
			continue;

		if (std::find(l.cbegin(), l.cend(), n) != l.cend()) //we found a loop
			continue;

		if (graph[n].type == "assign") {
			EdgeDescriptor p = pred(n, graph);
			c.emplace_back(graph[n].label);
			if (!graph[p].condition.empty())
				c.emplace_back(graph[p].condition);

			l.push_back(n);
			states.push(State{source(p,graph), l, c, w});
		} else if (graph[n].type == "phi") {
			std::cerr << "number of args " << graph[n].args.size() << std::endl;
			for (unsigned int i = 0 ; i < graph[n].args.size() ; i++) {
				std::vector<Constraint> newc{c};
				newc.emplace_back(graph[n].label.substr(0,graph[n].label.find(" = ")) + " = " + graph[n].args[i]);
				EdgeDescriptor p = pred(n, graph);
				if (!graph[p].condition.empty())
					newc.emplace_back(graph[p].condition);
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
					std::vector<Constraint> newc{c};
					if (!graph[*predi].condition.empty())
						newc.emplace_back(graph[*predi].condition);
					states.push(State{source(*predi,graph), std::move(newl), newc, -1});
				}
			}
		} else {
			std::cout << "possible path\n";
			std::transform(l.crbegin(), l.crend(),
					std::ostream_iterator<std::string>(std::cout, "\n"),
					[&graph](kayrebt::NodeDescriptor n) {
						return std::to_string(graph[n].id) +
						" (" + graph[n].label + ")";
					});
			std::cout << "\n";
			std::cout << "Associated constraints: \n";
			Constraint aggregate = Constraint::conjunct(c);
			std::cout <<  aggregate << std::endl;
			std::cout << "Yices says " << (Constraint::check_unsatisfiability(aggregate) ? "unsat :-)" : "sat :-(") << std::endl;
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
