#include <iostream>
#include <boost/graph/graphviz.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/graph/graph_utility.hpp>
#include <cstring>
#include <bcg_user.h>

#include "types.h"
#include "graphtoBCGtranslator.h"

using namespace std;
using namespace boost;

namespace kayrebt {

static char tool[] = "created by Kayrebt::Cleaner";

static void build_BCG_file_name(const std::string& file, char* newfilename)
{
	strncpy(newfilename,file.c_str(),file.length());
	unsigned long pos = file.find_last_of('.');
	if (pos != std::string::npos && file.substr(pos) == ".dot")
		newfilename[pos] = '\0';
	strncat(newfilename, ".bcg", 4);
}

}

int main(int argc, char** argv)
{
	if (argc < 2) {
		return 1;
	}

	kayrebt::GraphType graph;

	dynamic_properties dp;
	dp.property("id",    get(&kayrebt::Node::id, graph));
	dp.property("label", get(&kayrebt::Node::label, graph));
	dp.property("type",  get(&kayrebt::Node::type, graph));
	dp.property("line",  get(&kayrebt::Node::line, graph));
	dp.property("label", get(&kayrebt::Edge::condition, graph));
	dp.property("shape", get(&kayrebt::Node::shape, graph));
	dp.property("URL",   get(&kayrebt::Node::url, graph));
	ref_property_map<kayrebt::GraphType*,std::string> gfile(get_property(graph,&kayrebt::GraphAttr::file));
	dp.property("file",  gfile);
	ref_property_map<kayrebt::GraphType*,unsigned int> gline(get_property(graph,&kayrebt::GraphAttr::line));
	dp.property("line",  gline);

	std::ifstream dot(argv[1]);
	try {
	if (!read_graphviz(dot, graph, dp, "id")) {
		cerr << "FATAL: Could not parse a graphviz graph from " << argv[1] << endl;
		return 2;
	}
	dot.close();
	} catch (graph_exception& e) {
		cerr << "FATAL: The graph in " << dot << " could not be parsed: " << e.what() << endl;
	        return 2;
	}

	BCG_INIT();
	std::string file(argv[1]);
	char * filename = new char[file.size() + 5]; //make room for '.bcg\0'
	kayrebt::build_BCG_file_name(file,filename);
	BCG_IO_WRITE_BCG_BEGIN(filename, 0, 2, kayrebt::tool, BCG_FALSE);
	(kayrebt::GraphToBCGTranslator(graph))();
	BCG_IO_WRITE_BCG_END();
	delete[] filename;
	return 0;
}
