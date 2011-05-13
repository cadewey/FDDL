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

#include "mddhandle.h"

template < class T > class DynArray;
class Forest;

const unsigned int TABLE_SIZE = 1009;

class UniqueTable {
 private:
	const Forest *const m_forest;
	struct TableNode {
		level k;
		node_idx p;
		TableNode *next;
		TableNode() {
			next = NULL;
	        }
        };
	TableNode ***m_table;
	unsigned int m_numlevels;

 public:
	UniqueTable(const int K, const Forest * const forest);
	~UniqueTable();
	int look_up(const level k, const node_idx p) const;
	int add(const level k, const node_idx p);
	int remove(const level k, const node_idx p);
	int remap(const level k, DynArray < node_idx > *const transTable);
};
#endif				//__UNIQUE_TABLE_H
