/*
 * FDDL : The Free Decision Diagram Library 
 * Copyright (C) 2004 Robert Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

#include "uniquetable.h"

int
 uniquetable::LookUp(level k, node_idx p)
{
    table_node *cur;
    unsigned int idx;

    idx = hashfunc(k, p) % TABLE_SIZE;
    cur = table[k][idx];
    while (cur != NULL) {
	if (compare(k, p, cur->p) == 1) {
	    return cur->p;
	}
	cur = cur->next;
    }
    return -1;
}

int uniquetable::Add(level k, node_idx p)
{
    node_idx r;
    table_node *newNode;
    unsigned int idx;

    r = LookUp(k, p);
    if (r != -1)
	return r;

    idx = hashfunc(k, p) % TABLE_SIZE;

    newNode = new table_node;
    newNode->k = k;
    newNode->p = p;
    newNode->next = table[k][idx];
    table[k][idx] = newNode;
    return p;
}

int uniquetable::Delete(level k, node_idx p)
{
    table_node *cur;
    table_node *prev;
    unsigned int idx;

    idx = hashfunc(k, p) % TABLE_SIZE;

    prev = NULL;
    cur = table[k][idx];

    while (cur != NULL) {
	if (compare(k, p, cur->p) == 1) {
	    if (prev == NULL) {
		table[k][idx] = cur->next;
		delete cur;
	    } else {
		prev->next = cur->next;
		delete cur;
	    }
	    return 1;
	}
	prev = cur;
	cur = cur->next;
    }
    return 0;
}

int uniquetable::Remap(level k, Dynarray < node_idx > *transTable)
{
    int i;
    table_node *cur;
    node_idx newP;

    for (i = 0; i < TABLE_SIZE; i++) {
	cur = table[k][i];
	while (cur != NULL) {
	    newP = (*(*transTable)[cur->p]);
	    cur->p = newP;
	    cur = cur->next;
	}
    }
}
