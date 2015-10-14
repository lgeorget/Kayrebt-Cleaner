#ifndef GRAPH_TO_BCG_TRANSLATOR
#define GRAPH_TO_BCG_TRANSLATOR

#include <string>
#include <map>
#include <list>
#include <utility>
#include <bcg_user.h>

#include "types.h"

namespace kayrebt {

class GraphToBCGTranslator
{
    private:
        std::map<NodeDescriptor,BCG_TYPE_STATE_NUMBER> _visited;
        std::list<std::pair<NodeDescriptor,BCG_TYPE_STATE_NUMBER>> _nonVisited;
        GraphType _graph;
	std::string _bcgFile;
	unsigned int _index = 0;

	NodeDescriptor findInitNode();

    public:
	GraphToBCGTranslator(GraphType&& graph, std::string& bcgFile);
	void operator()();
};

}
#endif
