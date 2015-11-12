#include <boost/config.hpp>
#include <iostream>
#include <sys/stat.h>
#include <algorithm>

#include <boost/graph/graphviz.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/graph/graph_utility.hpp>

#include "types.h"
#include "decider.h"

using namespace boost;

void usage(const char* program, int exitCode)
{
	std::cerr << "Usage: "
		  << program << " <diagrams directory> <path to main diagram>"
		                " <output directory root>\n"
		  << "the <path to main diagram> is relative to "\
		     "<diagrams directory>"
		  << std::endl;
	exit(exitCode);
}

int main(int argc, char** argv)
{
	if (argc < 4)
		usage(argv[0], 1);

	std::string dir(argv[1]);
	if (dir[dir.length() - 1] != '/')
		dir += "/";
	std::string inputGraph(argv[2]);
	if (inputGraph.find(".dot",inputGraph.length() - 4) == std::string::npos)
		inputGraph += ".dot";
	std::string basepath(inputGraph);
	basepath.erase(basepath.find_last_of('/') + 1);
	std::string outputGraph(argv[3]);

	if (access(dir.c_str(), R_OK | X_OK) != 0) {
		std::cerr << "ERROR : dir \"" << dir << "\" does not exist"
			" or is not accessible." << std::endl;
		usage(argv[0], 2);
	}

	if (access((dir + inputGraph).c_str(), R_OK) != 0) {
		std::cerr << "ERROR : graph \"" << dir << inputGraph << "\" does not exist"
			" or is not accessible." << std::endl;
		usage(argv[0], 3);
	}

	Decider decider(dir,outputGraph);
	std::cerr << "Exploring " << inputGraph << std::endl;
	auto result = decider.decide(inputGraph);
	result.wait();

	decider.outputAllDiagrams(std::cout);
}
