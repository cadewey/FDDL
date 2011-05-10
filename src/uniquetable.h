/*
 * FDDL : The Free Decision Diagram Library Copyright (C) 2004 Robert
 * Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

#ifndef FDDL_UNIQUE_TABLE_H
#   define FDDL_UNIQUE_TABLE_H 1

#   include "mddtypes.h"
#   include "dynarray.h"

#   define TABLE_SIZE 1009
class   uniquetable {

	unsigned int (*hashfunc) (level, node_idx);
	int     (*compare) (level, node_idx, node_idx);
	class   table_node {
	 public:
		level   k;
		node_idx p;
		table_node *next;
		        table_node() {
			next = NULL;
	}};

	table_node ***table;
	int     numlevels;

 public:
	uniquetable(int K, unsigned int (*h) (level, node_idx),
					int (*c) (level, node_idx, node_idx)) {
		numlevels = K + 1;
		table = new table_node **[numlevels];
		for (int i = 0; i < numlevels; i++) {
			table[i] = new table_node *[TABLE_SIZE];
			for (int j = 0; j < TABLE_SIZE; j++) {
				table[i][j] = NULL;
			}
		}
		hashfunc = h;
		compare = c;
	}
	~uniquetable() {
		table_node *cur;

		for (int i = 0; i < numlevels; i++) {
			for (int j = 0; j < TABLE_SIZE; j++) {
				while (table[i][j] != NULL) {
					cur = table[i][j];
					table[i][j] = cur->next;
					delete  cur;
				}
			}
			delete[]table[i];
		}
		delete[]table;
	}
	int     LookUp(level k, node_idx p);
	int     Add(level k, node_idx p);
	int     Delete(level k, node_idx p);
	int     Remap(level k, dynarray < node_idx >*transTable);
};
#endif
