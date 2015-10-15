/**
 * \file BCGtographtranslator.h
 * \author Laurent Georget
 * \date 2015-10-15
 * \brief
 */
#ifndef BCG_TO_GRAPH_TRANSLATOR
#define BCG_TO_GRAPH_TRANSLATOR

#include <string>
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
        const GraphType& _graph;

    public:
	/**
	 * \brief 
	 *
	 * \param graph
	 */
	BCGToGraphTranslator(GraphType& graph);
	/**
	 * \brief 
	 */
	void operator()();
};

}
#endif
