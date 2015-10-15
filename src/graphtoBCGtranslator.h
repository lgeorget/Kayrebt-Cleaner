/**
 * \file graphToBCGtranslator.h
 * \author Laurent Georget
 * \date 2015-10-15
 * \brief
 */
#ifndef GRAPH_TO_BCG_TRANSLATOR
#define GRAPH_TO_BCG_TRANSLATOR

#include <string>
#include <map>
#include <list>
#include <utility>
#include <bcg_user.h>

#include "types.h"

namespace kayrebt {

/**
 * \brief 
 */
class GraphToBCGTranslator
{
    private:
	/**
	 * \brief 
	 */
        std::map<NodeDescriptor,BCG_TYPE_STATE_NUMBER> _visited;
	/**
	 * \brief 
	 */
        std::list<std::pair<NodeDescriptor,BCG_TYPE_STATE_NUMBER>> _nonVisited;
	/**
	 * \brief 
	 */
        const GraphType& _graph;
	/**
	 * \brief 
	 */
	unsigned int _index = 0;

	/**
	 * \brief 
	 *
	 * \return 
	 */
	NodeDescriptor findInitNode();

    public:
	/**
	 * \brief 
	 *
	 * \param graph
	 */
	GraphToBCGTranslator(const GraphType& graph);
	/**
	 * \brief 
	 */
	void operator()();
};

}
#endif
