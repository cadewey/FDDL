#ifndef __OPERATION_H
#define __OPERATION_H

#include "mddhandle.h"

class Forest;
class Cache;

class Operation{
	private:
		node_idx (*m_func)(level k, node_idx p, node_idx q);	
		Cache** m_cache;
		Forest* m_forest;
	public:	
		Operation(Forest* forest);
		~Operation();
		node_idx operation(level k, node_idx p, node_idx q);
};
#endif //__OPERATION_H
