/**
 * \file grapher.cpp
 * \author Laurent Georget
 * \date 2015-10-15
 * \brief
 */
#include <iostream>
#include <boost/graph/graphviz.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/graph/graph_utility.hpp>
#include <cstring>
#include <bcg_user.h>

#include "types.h"
#include "BCGtographtranslator.h"

using namespace std;
using namespace boost;

/**
 * \brief 
 */
namespace kayrebt {

	/**
	 * \brief 
	 */
static char tool[] = "created by Kayrebt::Cleaner";

/**
 * \brief 
 *
 * \param file
 * \param newfilename
 */
static std::string build_DOT_file_name(const char* newfilename)
{
	std::string file(newfilename);
	unsigned long pos = file.find_last_of('.');
	if (pos != std::string::npos && file.substr(pos) == ".bcg")
		file.replace(pos,4,".dot");
	else
		file.append(".dot");
	return file;
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
int main(int argc, char** argv)
{
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " <file.bcg>" << endl;
		return 1;
	}

	kayrebt::GraphType graph;

	dynamic_properties dp;
	dp.property("id",    get(&kayrebt::Node::id, graph));
	dp.property("label", get(&kayrebt::Node::label, graph));
	dp.property("label", get(&kayrebt::Edge::condition, graph));

	char * filename = new char[strlen(argv[1]) + 5]; //make room for the .bcg extension
	strncpy(filename,argv[1],strlen(argv[1]));

	std::string dotFile(kayrebt::build_DOT_file_name(filename));
	std::ofstream dot(dotFile);
	//exception not caught, nothing else than crashing is needed
	cerr << "Will read graph from " << filename << " and write to " << dotFile << endl;

	BCG_TYPE_OBJECT_TRANSITION bcg;
	BCG_INIT();
	BCG_OT_READ_BCG_BEGIN(filename,&bcg,0);
	(kayrebt::BCGToGraphTranslator(graph,bcg))();
	BCG_OT_READ_BCG_END(&bcg);
	write_graphviz_dp(dot,graph,dp,"id");
	delete[] filename;
	return 0;
}
