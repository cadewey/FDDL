/* 
 * FDDL : The Free Decision Diagram Library 
 * Copyright (C) 2004 Robert Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

#ifndef __CACHES_H
#define __CACHES_H 

#include "mddtypes.h"
const unsigned int INIT_SIZE=1009;

class Cache {

   private:
    struct cache_node {
	cache_node *m_next;
	node_idx p;
	node_idx q;
	node_idx s;
	node_idx r;

	cache_node() : p(-1), q(-1), s(-1), r(-1), m_next(NULL) {}
    };

  public:
    Cache() {
	m_list = new cache_node *[INIT_SIZE];
	m_size = INIT_SIZE;
	for (unsigned int i = 0; i < m_size; i++)
	    m_list[i] = NULL;
    }

    ~Cache() {
	    clear();
	    delete[] m_list;
    }

    void clear();
    node_idx hit(node_idx p, node_idx q = -1, node_idx s = -1);
    void add(node_idx r, node_idx p, node_idx q = -1, node_idx s = -1); 
  
  private:
    cache_node **m_list;
    int m_size;

};

class TupleCache {
    struct cache_node {
	cache_node *m_next;
	node_idx p;
	node_idx r;
	node_idx *vals;
	int numvals;
	 cache_node() : m_next(NULL) { }
    };

    cache_node **m_list;
    int m_size;

  public:
    TupleCache() {
	m_list = new cache_node *[INIT_SIZE];
	m_size = INIT_SIZE;
	for (int i = 0; i < m_size; i++) {
	    m_list[i] = NULL;
	}
    }

    ~TupleCache() {
	clear();
	delete[] m_list;
    }

    node_idx hit(node_idx * vals, int numvals);
    node_idx hit(node_idx p, node_idx * vals, int numvals);
    void add(node_idx r, node_idx p, node_idx * vals, int numvals);
    void add(node_idx r, node_idx * vals, int numvals);
    void clear();
};
#endif //__CACHES_H
