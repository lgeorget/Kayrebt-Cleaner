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
    std::map<NodeDescriptor,BCG_TYPE_STATE_NUMBER> _states;
	/**
	 * \brief 
	 */
    const GraphType& _graph;
	NodeDescriptor _initNode = GraphType::null_vertex();
	/**
	 * \brief 
	 */
	unsigned int _index = 1;

    inline void writeEdge(BCG_TYPE_STATE_NUMBER s, std::string l, BCG_TYPE_STATE_NUMBER t) {
        BCG_IO_WRITE_BCG_EDGE(s,const_cast<char*>(l.c_str()),t);
    }

    inline void writeEdge(BCG_TYPE_STATE_NUMBER s, const char* l, BCG_TYPE_STATE_NUMBER t) {
        BCG_IO_WRITE_BCG_EDGE(s,const_cast<char*>(l),t);
    }


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
