/*
 * FDDL : The Free Decision Diagram Library 
 * Copyright (C) 2004 Robert Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

#include "uniquetable.h"
#include "dynarray.h"
#include "mdd.h"

UniqueTable::UniqueTable(const int K,
			 const Forest * const forest):m_numlevels(K + 1),
m_forest(forest)
{
	m_table = new TableNode **[m_numlevels];
	for (unsigned int i = 0; i < m_numlevels; i++) {
		m_table[i] = new TableNode*[TABLE_SIZE];
		for (unsigned int j = 0; j < TABLE_SIZE; j++) {
			m_table[i][j] = NULL;
		}
	}
}

UniqueTable::~UniqueTable()
{
	TableNode *cur;

	for (unsigned int i = 0; i < m_numlevels; i++) {
		for (unsigned int j = 0; j < TABLE_SIZE; j++) {
			while (m_table[i][j] != NULL) {
				cur = m_table[i][j];
				m_table[i][j] = cur->next;
				delete cur;
			}
		}
		delete[]m_table[i];
	}
	delete[]m_table;
}

node_idx UniqueTable::look_up(const level k, const node_idx p) const {
	TableNode *cur;
	unsigned int idx;

	idx = m_forest->hashnode(k, p) % TABLE_SIZE;
	cur = m_table[k][idx];
	while (cur != NULL)
{
if (m_forest->compare(k, p, cur->p) == 1) {
	return cur->p;
}
cur = cur->next;
}
	return -1;
}

node_idx UniqueTable::add(const level k, const node_idx p)
{
	node_idx r;
	TableNode *newNode;
	unsigned int idx;

	r = look_up(k, p);
	if (r != -1)
		return r;

	idx = m_forest->hashnode(k, p) % TABLE_SIZE;

	newNode = new TableNode;
	newNode->k = k;
	newNode->p = p;
	newNode->next = m_table[k][idx];
	m_table[k][idx] = newNode;
	return p;
}

int UniqueTable::remove(const level k, const node_idx p)
{
	TableNode *cur;
	TableNode *prev;
	unsigned int idx;

	idx = m_forest->hashnode(k, p) % TABLE_SIZE;

	prev = NULL;
	cur = m_table[k][idx];

	while (cur != NULL) {
		if (m_forest->compare(k, p, cur->p) == 1) {
			if (prev == NULL) {
				m_table[k][idx] = cur->next;
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

int UniqueTable::remap(const level k, DynArray < node_idx > *const transTable)
{
	int i;
	TableNode *cur;
	node_idx newP;

	for (i = 0; i < TABLE_SIZE; i++) {
		cur = m_table[k][i];
		while (cur != NULL) {
			newP = (*(*transTable)[cur->p]);
			cur->p = newP;
			cur = cur->next;
		}
	}
}
