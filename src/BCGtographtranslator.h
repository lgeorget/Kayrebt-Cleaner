/**
 * \file BCGtographtranslator.h
 * \author Laurent Georget
 * \date 2015-10-15
 * \brief
 */
#ifndef BCG_TO_GRAPH_TRANSLATOR
#define BCG_TO_GRAPH_TRANSLATOR

#include <string>
#include <map>
#include <bcg_user.h>

#include "types.h"

namespace kayrebt {

/**
 * \brief 
 */
class BCGToGraphTranslator
{
    private:
	/**
	 * \brief 
	 */
        GraphType& _graph;
        const BCG_TYPE_OBJECT_TRANSITION& _bcg;
	unsigned int _index = 0;
	std::map<BCG_TYPE_STATE_NUMBER,NodeDescriptor> _nodes;

    public:
	/**
	 * \brief 
	 *
	 * \param graph
	 */
	BCGToGraphTranslator(GraphType& graph, const BCG_TYPE_OBJECT_TRANSITION& bcg);
	/**
	 * \brief 
	 */
	void operator()();
};

}
#endif
