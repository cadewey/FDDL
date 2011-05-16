#ifndef __OPERATION_H
#define __OPERATION_H

#include "mddhandle.h"

class Forest;
class Cache;

class Operation{
	protected:
		Cache** m_cache;
		Forest* m_forest;
	public:	
		Operation(Forest* forest);
		~Operation();
};
#endif //__OPERATION_H
