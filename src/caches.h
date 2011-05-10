/* FDDL : The Free Decision Diagram Library Copyright (C) 2004 Robert
 * Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

#ifndef FDDL_CACHES_H
#   define FDDL_CACHES_H 1
#   include "mddtypes.h"
#   define INIT_SIZE 1009

class cache {
	class   cache_node {
	 public:
		node_idx p;
		node_idx q;
		node_idx s;

		node_idx r;
		cache_node *next;
		        cache_node() {
			p = -1;
			q = -1;
			s = -1;
			r = -1;
			next = NULL;
	}};

	cache_node **list;
	int     size;

 public:
	cache() {
		list = new cache_node *[INIT_SIZE];
		size = INIT_SIZE;
		for (int i = 0; i < size; i++)
			list[i] = NULL;
	}
	~cache() {
/*		for (int i = 0; i < size; i++) {
			if (list[i] != NULL)
				delete  list[i];
		}
*/
		Clear();
		delete[] list;
	}
	node_idx Hit(node_idx p);
	node_idx Hit(node_idx p, node_idx q);
	node_idx Hit(node_idx p, node_idx q, node_idx s);
	void    Add(node_idx p, node_idx r);
	void    Add(node_idx p, node_idx q, node_idx r);
	void    Add(node_idx p, node_idx q, node_idx s, node_idx r);
	void    Clear();
};

class tuple_cache {
	class   cache_node {
	 public:
		node_idx p;
		node_idx *vals;
                int numvals;
		node_idx r;
		cache_node *next;
		        cache_node() {
			next = NULL;
	}};

	cache_node **list;
	int     size;

 public:
	tuple_cache() {
		list = new cache_node *[INIT_SIZE];
		size = INIT_SIZE;
		for (int i = 0; i < size; i++) {
			list[i] = NULL;
		}
	}

	~tuple_cache() {
		Clear();
/*		for (int i = 0; i < size; i++) {
			if (list[i] != NULL)
				delete  list[i];
		}
*/
		delete[]list;
	}

	node_idx Hit(node_idx *vals, int numvals);
	node_idx Hit(node_idx p, node_idx *vals, int numvals);
	void    Add(node_idx p, node_idx *vals, int numvals, node_idx r);
	void    Add(node_idx *vals, int numvals, node_idx r);
	void    Clear();
};
#endif
