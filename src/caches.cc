/*
 * FDDL : The Free Decision Diagram Library 
 * Copyright (C) 2004 Robert Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

#include "caches.h"

node_idx cache::Hit(node_idx p)
{
    int idx;
    cache_node *cur;

    assert(p >= 0);
    idx = p % size;
    cur = list[idx];
    while (cur != NULL) {
	if (cur->p == p)
	    return cur->r;
	cur = cur->next;
    }
    return -1;
}

node_idx cache::Hit(node_idx p, node_idx q)
{
    int idx;
    cache_node *cur;

    assert(p >= 0);
    assert(q >= 0);
    idx = (p * 256 + q) % size;
    cur = list[idx];
    while (cur != NULL) {
	if (cur->p == p && cur->q == q)
	    return cur->r;
	cur = cur->next;
    }
    return -1;
}

node_idx cache::Hit(node_idx p, node_idx q, node_idx s)
{
    int idx;
    cache_node *cur;

    assert(p >= 0);
    assert(q >= 0);
    assert(s >= 0);
    idx = (p * 256 * 256 + q * 256 + s) % size;
    cur = list[idx];
    while (cur != NULL) {
	if (cur->p == p && cur->q == q && cur->s)
	    return cur->r;
	cur = cur->next;
    }
    return -1;
}

void cache::Add(node_idx p, node_idx r)
{
    int idx;
    cache_node *newNode;

    newNode = new cache_node;
    newNode->p = p;
    newNode->r = r;

    idx = p % size;
    newNode->next = list[idx];
    list[idx] = newNode;
}

void cache::Add(node_idx p, node_idx q, node_idx r)
{
    int idx;
    cache_node *newNode;

    newNode = new cache_node;
    newNode->p = p;
    newNode->q = q;
    newNode->r = r;

    idx = (p * 256 + q) % size;
    newNode->next = list[idx];
    list[idx] = newNode;
}

void cache::Add(node_idx p, node_idx q, node_idx s, node_idx r)
{
    int idx;
    cache_node *newNode;

    newNode = new cache_node;
    newNode->p = p;
    newNode->q = q;
    newNode->s = s;
    newNode->r = r;

    idx = (p * 256 * 256 + q * 256 + s) % size;
    newNode->next = list[idx];
    list[idx] = newNode;
}

void cache::Clear()
{
    cache_node *prev;

    for (int i = 0; i < size; i++) {
	while (list[i] != NULL) {
	    prev = list[i];
	    list[i] = list[i]->next;
	    delete prev;
	}
    }
}

node_idx tuple_cache::Hit(node_idx p, node_idx * vals, int numvals)
{
    int idx;
    cache_node *cur;

    idx = p;
    for (int i = 0; i < numvals; i++) {
	idx *= 256;
	idx += vals[i];
	idx %= size;
    }
    cur = list[idx];
    while (cur != NULL) {
	if (cur->p == p) {
	    int i;

	    for (i = 0; i < numvals; i++) {
		if (vals[i] != cur->vals[i])
		    break;
	    }
	    if (i == numvals)
		return cur->r;
	}
	cur = cur->next;
    }
    return -1;
}

node_idx tuple_cache::Hit(node_idx * vals, int numvals)
{
    int idx;
    cache_node *cur;

    idx = 0;
    for (int i = 0; i < numvals; i++) {
	idx *= 256;
	idx += vals[i];
	idx %= size;
    }
    cur = list[idx];
    while (cur != NULL) {
	if (cur->numvals != numvals) {
	    cur = cur->next;
	    continue;
	}
	for (int i = 0; i < numvals; i++) {
	    if (cur->vals[i] != vals[i]) {
		cur = cur->next;
		continue;
	    }
	}
	return cur->r;
    }
    return -1;
}

void tuple_cache::Add(node_idx p, node_idx * vals, int numvals, node_idx r)
{
    int idx;
    cache_node *newNode;

    newNode = new cache_node;
    newNode->p = p;
    newNode->vals = new node_idx[numvals];
    newNode->numvals = numvals;

    for (int i = 0; i < numvals; i++)
	newNode->vals[i] = vals[i];
    newNode->r = r;

    idx = p % size;
    for (int i = 0; i < numvals; i++) {
	idx *= 256;
	idx += vals[i];
    }
    idx %= size;
    newNode->next = list[idx];
    list[idx] = newNode;
}

void tuple_cache::Add(node_idx * vals, int numvals, node_idx r)
{
    int idx;
    cache_node *newNode;

    newNode = new cache_node;
    newNode->p = 0;
    newNode->vals = new node_idx[numvals];

    for (int i = 0; i < numvals; i++)
	newNode->vals[i] = vals[i];
    newNode->r = r;

    idx = 0;
    for (int i = 0; i < numvals; i++) {
	idx *= 256;
	idx += vals[i];
    }
    idx %= size;
    newNode->next = list[idx];
    list[idx] = newNode;
}

void tuple_cache::Clear()
{
    cache_node *prev;

    for (int i = 0; i < size; i++) {
	while (list[i] != NULL) {
	    prev = list[i];
	    list[i] = list[i]->next;
	    delete prev;
	}
    }
}
