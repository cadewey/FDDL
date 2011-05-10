/*
 * FDDL : The Free Decision Diagram Library 
 * Copyright (C) 2004 Robert Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

#ifndef __UNIQUE_TABLE_H
#define __UNIQUE_TABLE_H

#include "mddtypes.h"
template <class T> class DynArray;

const unsigned int TABLE_SIZE = 1009;

typedef unsigned int (*hfunc)(level, node_idx);
typedef int (*cfunc)(level, node_idx, node_idx);

class UniqueTable {
	private:
		hfunc m_hashfunc;
		cfunc m_compare;
		struct table_node {
			level k;
			node_idx p;
			table_node *next;
			table_node() {
			    next = NULL;
		        }
		    };
		table_node ***m_table;
		unsigned int m_numlevels;

  public:
    UniqueTable(int K, hfunc h, cfunc c);
    ~UniqueTable();
    int look_up(level k, node_idx p);
    int add(level k, node_idx p);
    int remove(level k, node_idx p);
    int remap(level k, DynArray < node_idx > *transTable);
};
#endif //__UNIQUE_TABLE_H
