/**
 * \file BCGtographtranslator.cpp
 * \author Laurent Georget
 * \date 2015-10-15
 * \brief
 */
#include <iostream>
#include <boost/graph/graphviz.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graph_traits.hpp>
#include <string>
#include <bcg_user.h>

#include "BCGtographtranslator.h"
#include "types.h"

using namespace boost;
using namespace std;

namespace kayrebt
{
	BCGToGraphTranslator::BCGToGraphTranslator(GraphType& graph) : _graph(graph)
	{
	}

	void BCGToGraphTranslator::operator()()
	{
	}

}
