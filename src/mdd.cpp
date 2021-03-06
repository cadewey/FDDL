/*
 * FDDL : The Free Decision Diagram Library 
 * Copyright (C) 2004 Robert Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

//#define QUICKDEBUG
//#define NOCOMPACT
//#define FDDL_DEBUG
//#define MINDEBUG
//#define COMPACT_DEBUG

#ifdef FDDL_DEBUG
#define MINDEBUG
#endif

#include "mdd.h"
#include "uniquetable.h"
#include <stdio.h>

int compactions;
int numnodes = 0;
int peaknodes = 0;
static float numSolutions;

Forest::Forest(int numlevels, int *maxvals)
{
    CompactCounter = 0;
    sparseEnabled = true;
    K = numlevels - 1;
    maxVals = new int[K + 1];

    for (level k = 0; k <= K; k++)
	maxVals[k] = maxvals[k];

    //Initialize structures for a dynamic array of MDD nodes and arcs
    //for each level of the forest.

    nodes = new DynArray < Node > *[K + 1];
    arcs = new DynArray < node_idx > *[K + 1];
    labels = new DynArray < Label * >*[K + 1];

    for (int k = 1; k <= K; k++) {
	nodes[k] = new DynArray < Node >;
	arcs[k] = new DynArray < node_idx > (0);
	labels[k] = new DynArray < Label * >(0);
    }

    //Keep track of the last used position of the node and arc arrays
    //respectively.

    last = new node_idx[K + 1];
    tail = new int[K + 1];

    for (int k = 1; k <= K; k++) {
	last[k] = 1;		//Node 0 is reserved.
	tail[k] = 0;
    }

    //Initialize caches for common operations

    MaxCache = new Cache *[K + 1];
    ProjectCache = new Cache *[K + 1];
    PruneCache = new Cache *[K + 1];
    RestrictCache = new Cache *[K + 1];
    MinCache = new Cache *[K + 1];
    ComplementCache = new Cache *[K + 1];
    BComplementCache = new Cache *[K + 1];
    ValRestrictCache = new Cache *[K + 1];
    LessThanCache = new Cache *[K + 1];
    CombineCache = new Cache *[K + 1];
    ReplaceCache = new Cache *[K + 1];
    ProjectOntoCache = new Cache *[K + 1];
    ReplaceStrictCache = new Cache *[K + 1];
    PrintCache = new Cache *[K + 1];
    ShiftCache = new Cache *[K + 1];
    ApplyCache = new TupleCache *[K + 1];
    SelectCache = new TupleCache *[K + 1];

    for (int k = 1; k <= K; k++) {
	FDDL_NODE(k, 0).in = 0;
	FDDL_NODE(k, 0).size = 0;
	FDDL_NODE(k, 0).flags = FLAG_NONE;
	FDDL_NODE(k, 0).down = 0;
	ProjectCache[k] = new Cache;
	PruneCache[k] = new Cache;
	RestrictCache[k] = new Cache;
	MaxCache[k] = new Cache;
	MinCache[k] = new Cache;
	ComplementCache[k] = new Cache;
	BComplementCache[k] = new Cache;
	ValRestrictCache[k] = new Cache;
	LessThanCache[k] = new Cache;
	CombineCache[k] = new Cache;
	ReplaceCache[k] = new Cache;
	ProjectOntoCache[k] = new Cache;
	ReplaceStrictCache[k] = new Cache;
	ShiftCache[k] = new Cache;
	PrintCache[k] = new Cache;
	ApplyCache[k] = new TupleCache;
	SelectCache[k] = new TupleCache;
    }

    //Create a hashtable of K levels to act as the Unique Table
    UT = new UniqueTable(K, this);
    garbage_alg = O_LAZY;
    garbage_threshold = 1;
}

Forest::~Forest()
{
    for (level k = K; k > 0; k--) {
	if (nodes[k])
	    delete nodes[k];

	if (arcs[k])
	    delete arcs[k];

	if (labels[k])
	    delete labels[k];

	if (ProjectCache[k])
	    delete ProjectCache[k];

	if (PruneCache[k])
	    delete PruneCache[k];

	if (RestrictCache[k])
	    delete RestrictCache[k];

	if (MaxCache[k])
	    delete MaxCache[k];

	if (MinCache[k])
	    delete MinCache[k];

	if (ComplementCache[k])
	    delete ComplementCache[k];

	if (BComplementCache[k])
	    delete BComplementCache[k];

	if (ValRestrictCache[k])
	    delete ValRestrictCache[k];

	if (LessThanCache[k])
	    delete LessThanCache[k];

	if (ApplyCache[k])
	    delete ApplyCache[k];

	if (CombineCache[k])
	    delete CombineCache[k];

	if (ReplaceCache[k])
	    delete ReplaceCache[k];

	if (ProjectOntoCache[k])
	    delete ProjectOntoCache[k];

	if (ReplaceStrictCache[k])
	    delete ReplaceStrictCache[k];

	if (SelectCache[k])
	    delete SelectCache[k];

	if (ShiftCache[k])
	    delete ShiftCache[k];

	if (PrintCache[k])
	    delete PrintCache[k];
    }

    delete[]ProjectCache;
    delete[]PruneCache;
    delete[]RestrictCache;
    delete[]MaxCache;
    delete[]MinCache;
    delete[]ComplementCache;
    delete[]BComplementCache;
    delete[]ValRestrictCache;
    delete[]ApplyCache;
    delete[]LessThanCache;
    delete[]CombineCache;
    delete[]ReplaceCache;
    delete[]ProjectOntoCache;
    delete[]ReplaceStrictCache;
    delete[]ShiftCache;
    delete[]SelectCache;
    delete[]PrintCache;
    delete[]arcs;
    delete[]labels;
    delete[]nodes;
    delete[]maxVals;
    delete[]last;
    delete[]tail;
    delete UT;
}

unsigned int Forest::hashnode(const level k, const node_idx p) const
{
    Node *nodeP;
    unsigned int val;

    nodeP = &FDDL_NODE(k, p);
    if (nodeP->size == 0)
	return 0;
    val = FDDL_ARC(k, nodeP, 0);
    for (arc_idx i = 1; i <= maxVals[k]; i++) {
	val *= 256;
	if (i < nodeP->size)
	    val += FDDL_ARC(k, nodeP, i);
    }
    return val;
}

int Forest::compare(const level k, const node_idx p, const node_idx q) const
{
    Node *nodeP;
    Node *nodeQ;

    nodeP = &FDDL_NODE(k, p);
    nodeQ = &FDDL_NODE(k, q);
    if (nodeP->size != nodeQ->size)
	return 0;
    for (arc_idx i = 0; i < nodeP->size; i++) {
	if (FDDL_ARC(k, nodeP, i) != FDDL_ARC(k, nodeQ, i))
	    return 0;
    }
    return 1;
}

void Forest::ToggleSparsity(bool SparseSwitch)
{
    sparseEnabled = SparseSwitch;
}

//Should we collect garbage?
//Returns true if we're using Strict and have hit the threshold
//False otherwise.  This function can be easily modified for use with
//New Garbage Collection Schemes.

void Forest::SetGarbageCollection(int alg, int threshold)
{
    garbage_alg = alg;
    garbage_threshold = threshold;
}

//Simple Recursive Maximum of <k,p> and <k,q>

node_idx Forest::InternalMax(level k, node_idx p, node_idx q)
{
    //Easy Terminal Cases
    if (p == 0 || p == q)
	return q;
    if (q == 0)
	return p;
    if (k == 0)
	return p > q ? p : q;

    //Check for an entry in the Cache.
    node_idx result;

    result = MaxCache[k]->hit(p, q);
    if (result >= 0) {
	if (!(FDDL_NODE(k, result).flags & FLAG_DELETED))
	    return result;
	if (garbage_alg != O_STRICT)
	    return CheckIn(k, result);
    }

    result = NewNode(k);	//result initially has size 0.
    Node *nodeP = &FDDL_NODE(k, p);
    Node *nodeQ = &FDDL_NODE(k, q);

    int psize = nodeP->size;
    int qsize = nodeQ->size;

    //If neither node is sparse, do things the easy way.
    if (!IS_SPARSE(nodeP) && !IS_SPARSE(nodeQ)) {
	for (arc_idx i = 0; i < (psize > qsize ? psize : qsize); i++) {
	    node_idx u = InternalMax(k - 1,
				     i < psize ? FULL_ARC(k, nodeP, i) : 0,
				     i < qsize ? FULL_ARC(k, nodeQ,
							  i) : 0);

	    SetArc(k, result, i, u);
	}
    } else if (IS_SPARSE(nodeP) && IS_SPARSE(nodeQ)) {
	//If both nodes are sparse, do things the fast way!
	//Scan from left to right.  If i is the smaller value, put it in the
	//node.  If j is the smaller value, put it in the node.  If i==j, put
	//the union of i and j in the node.  

	for (arc_idx i = 0, j = 0; i < psize || j < qsize;) {
	    arc_idx pdx = SPARSE_INDEX(k, nodeP, i);
	    node_idx pval = SPARSE_ARC(k, nodeP, i);
	    arc_idx qdx = SPARSE_INDEX(k, nodeQ, j);
	    node_idx qval = SPARSE_ARC(k, nodeQ, j);

	    if (i >= psize) {
		SetArc(k, result, qdx, qval);
		j++;
	    } else if (j >= qsize) {
		SetArc(k, result, pdx, pval);
		i++;
	    } else if (pdx < qdx) {
		SetArc(k, result, pdx, pval);
		i++;
	    } else if (qdx < pdx) {
		SetArc(k, result, qdx, qval);
		j++;
	    } else {
		SetArc(k, result, pdx, InternalMax(k - 1, pval, qval));
		i++;
		j++;
	    }
	}
    } else {
	if (IS_SPARSE(nodeP) && !IS_SPARSE(nodeQ)) {
	    int j = 0;

	    for (int i = 0; i < nodeP->size || j < nodeQ->size;) {
		int idx = SPARSE_INDEX(k, nodeP, i);
		int ival = SPARSE_ARC(k, nodeP, i);
		int jval = FULL_ARC(k, nodeQ, j);

		if (i >= nodeP->size) {
		    SetArc(k, result, j, jval);
		    j++;
		} else if (j >= nodeQ->size) {
		    SetArc(k, result, idx, ival);
		    i++;
		} else if (j < idx) {
		    SetArc(k, result, j, jval);
		    j++;
		} else if (idx < j) {
		    SetArc(k, result, idx, ival);
		    i++;
		} else {
		    SetArc(k, result, j, InternalMax(k - 1, ival, jval));
		    i++;
		    j++;
		}
	    }
	} else if (IS_SPARSE(nodeQ) && !IS_SPARSE(nodeP)) {
	    int j = 0;

	    for (int i = 0; i < nodeP->size || j < nodeQ->size;) {
		int jdx = SPARSE_INDEX(k, nodeQ, j);
		int jval = SPARSE_ARC(k, nodeQ, j);
		int ival = FULL_ARC(k, nodeP, i);

		if (i >= nodeP->size) {
		    SetArc(k, result, jdx, jval);
		    j++;
		} else if (j >= nodeQ->size) {
		    SetArc(k, result, i, ival);
		    i++;
		} else if (i < jdx) {
		    SetArc(k, result, i, ival);
		    i++;
		} else if (jdx < i) {
		    SetArc(k, result, jdx, jval);
		    j++;
		} else {
		    SetArc(k, result, i, InternalMax(k - 1, ival, jval));
		    i++;
		    j++;
		}
	    }

	}
    }

    node_idx newresult = CheckIn(k, result);

//  if (k > 0 && newresult)
//    FDDL_NODE (k, newresult).flags |= FLAG_CHECKED_IN;
    MaxCache[k]->add(newresult, p, q);
    MaxCache[k]->add(newresult, q, p);
    MaxCache[k]->add(newresult, p, newresult);
    MaxCache[k]->add(newresult, q, newresult);
    return newresult;
}

node_idx Forest::InternalRestrict(level k, node_idx p, node_idx q)
{
    //Easy Terminal Cases
    if (p == 0 || p == q)
	return q;
    if (q == 0)
	return p;
    if (k == 0)
	return q;

    //Check for an entry in the Cache.
    node_idx result;

    result = RestrictCache[k]->hit(p, q);
    if (result >= 0) {
	if (!(FDDL_NODE(k, result).flags & FLAG_DELETED))
	    return result;
	if (garbage_alg != O_STRICT)
	    return CheckIn(k, result);
    }

    result = NewNode(k);	//result initially has size 0.
    Node *nodeP = &FDDL_NODE(k, p);
    Node *nodeQ = &FDDL_NODE(k, q);

    int psize = nodeP->size;
    int qsize = nodeQ->size;

    //If neither node is sparse, do things the easy way.
    if (!IS_SPARSE(nodeP) && !IS_SPARSE(nodeQ)) {
	for (arc_idx i = 0; i < (psize > qsize ? psize : qsize); i++) {
	    node_idx
		u = InternalRestrict(k - 1,
				     i < psize ? FULL_ARC(k, nodeP, i) : 0,
				     i < qsize ? FULL_ARC(k, nodeQ,
							  i) : 0);

	    SetArc(k, result, i, u);
	}
    } else if (IS_SPARSE(nodeP) && IS_SPARSE(nodeQ)) {
	//If both nodes are sparse, do things the fast way!
	//Scan from left to right.  If i is the smaller value, put it in the
	//node.  If j is the smaller value, put it in the node.  If i==j, put
	//the union of i and j in the node.  

	for (arc_idx i = 0, j = 0; i < psize || j < qsize;) {
	    arc_idx pdx = SPARSE_INDEX(k, nodeP, i);
	    node_idx pval = SPARSE_ARC(k, nodeP, i);
	    arc_idx qdx = SPARSE_INDEX(k, nodeQ, j);
	    node_idx qval = SPARSE_ARC(k, nodeQ, j);

	    if (i >= psize) {
		SetArc(k, result, qdx, qval);
		j++;
	    } else if (j >= qsize) {
		SetArc(k, result, pdx, pval);
		i++;
	    } else if (pdx < qdx) {
		SetArc(k, result, pdx, pval);
		i++;
	    } else if (qdx < pdx) {
		SetArc(k, result, qdx, qval);
		j++;
	    } else {
		SetArc(k, result, pdx,
		       InternalRestrict(k - 1, pval, qval));
		i++;
		j++;
	    }
	}
    } else {
	if (IS_SPARSE(nodeP) && !IS_SPARSE(nodeQ)) {
	    int j = 0;

	    for (int i = 0; i < nodeP->size || j < nodeQ->size;) {
		int idx = SPARSE_INDEX(k, nodeP, i);
		int ival = SPARSE_ARC(k, nodeP, i);
		int jval = FULL_ARC(k, nodeQ, j);

		if (i >= nodeP->size) {
		    SetArc(k, result, j, jval);
		    j++;
		} else if (j >= nodeQ->size) {
		    SetArc(k, result, idx, ival);
		    i++;
		} else if (j < idx) {
		    SetArc(k, result, j, jval);
		    j++;
		} else if (idx < j) {
		    SetArc(k, result, idx, ival);
		    i++;
		} else {
		    SetArc(k, result, j,
			   InternalRestrict(k - 1, ival, jval));
		    i++;
		    j++;
		}
	    }
	} else if (IS_SPARSE(nodeQ) && !IS_SPARSE(nodeP)) {
	    int j = 0;

	    for (int i = 0; i < nodeP->size || j < nodeQ->size;) {
		int jdx = SPARSE_INDEX(k, nodeQ, j);
		int jval = SPARSE_ARC(k, nodeQ, j);
		int ival = FULL_ARC(k, nodeP, i);

		if (i >= nodeP->size) {
		    SetArc(k, result, jdx, jval);
		    j++;
		} else if (j >= nodeQ->size) {
		    SetArc(k, result, i, ival);
		    i++;
		} else if (i < jdx) {
		    SetArc(k, result, i, ival);
		    i++;
		} else if (jdx < i) {
		    SetArc(k, result, jdx, jval);
		    j++;
		} else {
		    SetArc(k, result, i,
			   InternalRestrict(k - 1, ival, jval));
		    i++;
		    j++;
		}
	    }

	}
    }

    node_idx newresult = CheckIn(k, result);

//  if (k > 0 && newresult)
//    FDDL_NODE (k, newresult).flags |= FLAG_CHECKED_IN;
    RestrictCache[k]->add(newresult, p, q);
    return newresult;
}

//Ensure that node <k,p> is unique by inserting it into the Unique
//Table.  Also, determine whether it should be stored sparsely or in
//full and convert it if necessary.  <k,p> should be the LAST node at
//level k, so that we can reclaim space in the arc and node arrays.

node_idx Forest::CheckIn(level k, node_idx p)
{
    Node *nodeP;

    nodeP = &FDDL_NODE(k, p);
    //are sparse.
    if (nodeP->size == 0) {
	assert(!(nodeP->flags & FLAG_CHECKED_IN));
	DeleteNode(k, p);
	//If the node is a dummy (has size 0), it must be a just created
	//node, so it appears at the end of the node array for this level.
	//Therefore, we can save space by backing last[k] up one.
	assert(p == last[k] - 1);
	last[k]--;
	return 0;
    }
    assert(!(nodeP->flags & FLAG_SPARSE));
    if (sparseEnabled) {
	//Count the number of non zeros to see if we should store
	//in sparse format.

	int nnz = 0;

	for (arc_idx i = 0; i < nodeP->size; i++)
	    if (FULL_ARC(k, nodeP, i) != 0)
		nnz++;

	assert(nnz > 0);	//Otherwise, we should have deleted above
	arc_idx *tempArray = new arc_idx[2 * nnz + 1];

	//If there are few enough non-zeros, convert to sparse.
	if (nnz < (nodeP->size / 2) + 1) {
	    int current = 0;

	    for (arc_idx i = 0; i < nodeP->size; i++) {
		if (FULL_ARC(k, nodeP, i) != 0) {
		    tempArray[2 * current] = i;
		    tempArray[2 * current + 1] = FULL_ARC(k, nodeP, i);
		    current++;
		}
	    }
	    assert(current == nnz);
	    nodeP->flags = (flag)(nodeP->flags | FLAG_SPARSE);
	    for (arc_idx i = 0; i < nnz; i++) {
		SPARSE_INDEX(k, nodeP, i) = tempArray[2 * i];
		SPARSE_ARC(k, nodeP, i) = tempArray[2 * i + 1];
	    }
	    if (tail[k] == nodeP->size + nodeP->down) {
		tail[k] -= nodeP->size;
		tail[k] += (2 * nnz);
		nodeP->size = nnz;
	    }
	}
	delete[]tempArray;
    }

    node_idx q;

    q = UT->add(k, p);

    Node *nodeQ;
    nodeQ = &FDDL_NODE(k, q);

    if (q != p) {
	if (nodeP->flags & FLAG_CHECKED_IN && !(nodeQ->flags & FLAG_CHECKED_IN))
	    nodeQ->flags = (flag)(nodeQ->flags | FLAG_CHECKED_IN);

	for (int i = 0; i < nodeP->size; i++)
	    SetArc(k, p, i, 0);

	if (tail[k] == nodeP->size + nodeP->down) {
	    tail[k] -= nodeP->size;
	}

	DeleteNode(k, p);

	if (nodeP->flags & FLAG_CHECKED_IN) {	//Deleted Node is no longer checked in (we've scrambled the arcs!).
	    nodeP->flags = (flag)(nodeP->flags - FLAG_CHECKED_IN);
	}
	last[k]--;
    }
    if (nodeQ->flags & FLAG_DELETED) {
	nodeQ->flags = (flag)(nodeQ->flags - FLAG_DELETED);
	numnodes++;
    }
    assert(nodeQ->size > 0);
    return q;
}

//Create a new node at level k, and initialize its size and downpointer index.
//Only one new (unchecked-in) node should exist at a particular level.

node_idx Forest::NewNode(level k)
{
    Node *nodeP;

    nodeP = &FDDL_NODE(k, last[k]);
    nodeP->down = tail[k];
    nodeP->size = 0;
    nodeP->flags = FLAG_NONE;
    last[k]++;

    numnodes++;
    if (numnodes > peaknodes)
	peaknodes = numnodes;
    return last[k] - 1;
}

//Mark Node node <k,p> deleted
void Forest::DeleteNode(level k, node_idx p)
{
    if (k < 1)			//Can't delete terminal nodes!
	return;

    if (p == 0)			//Can't delete node 0!
	return;

    Node *nodeP;

    nodeP = &FDDL_NODE(k, p);

    if (nodeP->flags & FLAG_DELETED)	//If already deleted, return.
	return;

    assert(nodeP->in == 0);	//Only delete disconnected nodes!

    numnodes--;
    nodeP->flags = (flag)(nodeP->flags | FLAG_DELETED);
}

void Forest::DeleteDownstream(level k, node_idx p)
{
    if (p == 0)
	return;
    Node *nodeP = &FDDL_NODE(k, p);

    if (nodeP->flags & FLAG_DELETED)
	return;
    numnodes--;
    nodeP->flags = (flag)(nodeP->flags | FLAG_DELETED);
    UT->remove(k, p);
    for (int i = 0; i < nodeP->size; i++) {
	SetArc(k, p, i, 0);
    }
}

//Compact level K.

void Forest::CompactTopLevel()
{
    DynArray < node_idx > *arc_temp_array;	//Store the new arcs for level K
    arc_temp_array = new DynArray < node_idx > (0);
    node_idx i;
    arc_idx j;

    tail[K] = 0;		//Clear the arc array
    int numvalidnodes = 1;	//Start at 1, because 0 is valid

#ifdef BRIEF_DEBUG
    printf("Compact Top Level\n");
#endif

    for (i = 1; i < last[K]; i++)
	UT->remove(K, i);
    for (i = 1; i < last[K]; i++) {	//Scan throught the NODE list (except node 0)
	Node *nodeI = &FDDL_NODE(K, i);

	if (!IS_DELETED(nodeI)) {	//If it's not deleted,
	    //Copy (and compact) the arcs.
	    int newdown = tail[K];

	    if (nodeI->flags & FLAG_SPARSE) {	//If it's sparse, compact
		for (j = 0; j < nodeI->size; j++) {	//its arcs this way.
		    (*(*arc_temp_array)[tail[K]]) =
			SPARSE_INDEX(K, nodeI, j);
		    (*(*arc_temp_array)[tail[K] + 1]) =
			SPARSE_ARC(K, nodeI, j);
		    tail[K] += 2;
		}
	    } else {		//Otherwise, do it in full.
		for (j = 0; j < nodeI->size; j++) {
		    (*(*arc_temp_array)[tail[K]]) = FULL_ARC(K, nodeI, j);
		    tail[K]++;
		}
	    }
	    nodeI->down = newdown;
	    UT->add(K, i);
	    numvalidnodes++;
	}
	//If it IS deleted, all we do is take it out of the unique table.  
	//It should have been "deleted downstream" earlier. 
    }
    delete arcs[K];

    arcs[K] = arc_temp_array;
    FlushCaches(K);
}

void Forest::Compact(level k)
{
#ifdef BRIEF_DEBUG
    printf("Compact Level %d\n", k);
#endif
    compactions++;

#ifdef NOCOMPACT
    return;
#endif

    //Don't compact bad levels
    if (k > K)
	return;

    if (k == K) {		//The Top Level is special.
	CompactTopLevel();
	return;
    }

    DynArray < node_idx > *arc_temp_array;	//Store the new arcs for level k
    arc_temp_array = new DynArray < node_idx > (0);

    DynArray < Node > *node_temp_array;
    node_temp_array = new DynArray < Node >;	//Store the nodes here

    //Store a mapping of old->new of node indices so that we can re-hash and also update the level above.   
    
    DynArray<node_idx> *node_remap_array = new DynArray < node_idx > (0);

    node_idx i;
    arc_idx j;
    arc_idx arc;

    tail[k] = 0;		//Clear the arc array

    int numvalidnodes = 1;	//Start at 1, because 0 is valid
    (*(*node_remap_array)[0]) = 0;

    for (i = 1; i < last[k]; i++) {	//Scan throught the NODE list (except node 0)
	Node *nodeI = &FDDL_NODE(k, i);

	//If it's not deleted, copy it.
//      if (nodeI->in > 0)
	if (!(nodeI->flags & FLAG_DELETED)) {
	    //Save the "old" idx of the node
	    //into a temp array.  
	    (*(*node_remap_array)[i]) = numvalidnodes;
	    (*(*node_temp_array)[numvalidnodes]) = (*nodeI);

	    //Copy (and compact) the arcs, too.
	    int newdown = tail[k];

	    if (nodeI->flags & FLAG_SPARSE) {
		//If it's sparse, compact
		//its arcs this way.
		for (j = 0; j < nodeI->size; j++) {
		    (*(*arc_temp_array)[tail[k]]) =
			SPARSE_INDEX(k, nodeI, j);
		    (*(*arc_temp_array)[tail[k] + 1]) =
			SPARSE_ARC(k, nodeI, j);
		    tail[k] += 2;
		}
	    } else {		//Otherwise, do it in full.
		for (j = 0; j < nodeI->size; j++) {
		    (*(*arc_temp_array)[tail[k]]) = FULL_ARC(k, nodeI, j);
		    tail[k]++;
		}
	    }
	    (*(*node_temp_array)[numvalidnodes]).down = newdown;
	    numvalidnodes++;
	} else {		//If it IS deleted, "DeleteDownstream" it.
	    UT->remove(k, i);
	    assert(nodeI->flags & FLAG_DELETED);
	    if (k >= 2) {
		for (j = 0; j < nodeI->size; j++) {
		    if (nodeI->flags & FLAG_SPARSE)
			arc = SPARSE_ARC(k, nodeI, j);
		    else
			arc = FULL_ARC(k, nodeI, j);
		    Node *nodeJ = &FDDL_NODE(k - 1, arc);
		    SetArc(k, i, j, 0);	//Compaction should ALWAYS be done
		    if (nodeJ->in == 0)	//Top Down so that Deletion Marking
			DeleteNode(k - 1, arc);	//Actually Deletes -Downstream-
		}
	    }
	}
    }
    last[k] = numvalidnodes;
    delete nodes[k];

    nodes[k] = node_temp_array;

    UT->remap(k, node_remap_array);	//Update all the unique table entries

    delete arcs[k];

    arcs[k] = arc_temp_array;

    for (i = 1; i < last[k + 1]; i++) {	//Now fix our upstairs pointers.       
	Node *nodeI = &FDDL_NODE(k + 1, i);

	if (!IS_DELETED(nodeI)) {
	    if (nodeI->flags & FLAG_SPARSE) {
		for (j = 0; j < nodeI->size; j++) {
		    assert((*(*node_remap_array)
			    [SPARSE_ARC(k + 1, nodeI, j)]) >= 0);
		    SPARSE_ARC(k + 1, nodeI, j) = (*(*node_remap_array)
						   [SPARSE_ARC
						    (k + 1, nodeI, j)]);
		}
	    } else {
		for (j = 0; j < nodeI->size; j++) {
		    assert((*(*node_remap_array)
			    [FULL_ARC(k + 1, nodeI, j)]) >= 0);
		    FULL_ARC(k + 1, nodeI, j) = (*(*node_remap_array)
						 [FULL_ARC
						  (k + 1, nodeI, j)]);
		}
	    }
	}
    }

    //Reinsert nodes into Unique table.  Check that this does not violate 
    //integrity with an assert.
    for (i = 1; i < last[k + 1]; i++) {
	Node *nodeI = &FDDL_NODE(k + 1, i);

	if ((nodeI->flags & FLAG_CHECKED_IN) && !IS_DELETED(nodeI)) {
	    int s = UT->add(k + 1, i);
	    //@@@DEBUG: Removed for testing. @@@//
	    assert(s == i);
	}
    }
    FlushCaches(k);
    delete node_remap_array;
}

//Flush all caches associated with level k.
void Forest::FlushCaches(level k)
{
    MaxCache[k]->clear();
    RestrictCache[k]->clear();
    ReplaceCache[k]->clear();
    SelectCache[k]->clear();
    ComplementCache[k]->clear();
    BComplementCache[k]->clear();
    MinCache[k]->clear();
    PrintCache[k]->clear();
}

//Point arc <k,p>[i] at node <k-1,j>
//Obviously, k must be between 1 and K, inclusive, and p must refer to
//a valid node of the MDD.
void Forest::SetArc(level k, node_idx p, arc_idx i, node_idx j)
{
    Node *nodeP, *nodeOld, *nodeJ;

    nodeP = &FDDL_NODE(k, p);
    arc_idx old;
    arc_idx arc_to_clear;

    //If the old arc went somewhere, remove it.

    if (i >= nodeP->size)
	old = 0;
    else {
	if (nodeP->flags & FLAG_SPARSE)
	    old = SPARSE_ARC(k, nodeP, i);
	else
	    old = FULL_ARC(k, nodeP, i);
    }
    if (old == j)
	return;

    if (k > 1 && old > 0) {
	nodeOld = &FDDL_NODE(k - 1, old);
	nodeOld->in--;
    }
    if (i >= nodeP->size) {	//If we're extending a node (in place)
	assert(!(nodeP->flags & FLAG_SPARSE));	//Sparse nodes should 
	//never be extended. 

	for (arc_to_clear = nodeP->size; arc_to_clear < i; arc_to_clear++) {
	    //If we are extending the node, we need to set the arcs between
	    //the current last arc and the new arc to 0.
	    FULL_ARC(k, nodeP, arc_to_clear) = 0;
	}
	//Since we've extended the node, we have added more arcs to this
	//level and must update tail.
	tail[k] += (i + 1 - nodeP->size);
	nodeP->size = i + 1;
    }
    //Fix the incoming arc count for node <k-1,j>  
    if (k > 1 && j > 0) {
	nodeJ = &FDDL_NODE(k - 1, j);
	nodeJ->in++;
    }
    //Set the arc
    if (nodeP->flags & FLAG_SPARSE)
	(*(*arcs[k])[nodeP->down + 2 * i + 1]) = j;
    else
	(*(*arcs[k])[nodeP->down + i]) = j;
}

//Store, in full, the arcs of the sparse node <k,p> in the array fullarray
//and return the size of the resulting array.
int Forest::UnpackNode(level k, arc_idx p, int *&fullarray)
{
#ifdef FDDL_DEBUG
    printf("UnpackNode(%d, %d)\n", k, p);
#endif
    int psize;
    int i;
    Node *nodeP;

    nodeP = &FDDL_NODE(k, p);
    psize = 0;
    for (i = 0; i < nodeP->size; i++) {
	int idx = SPARSE_INDEX(k, nodeP, i);

	if (idx >= psize)
	    psize = idx + 1;
    }
    //    psize = SPARSE_INDEX(k,nodeP,(nodeP->size-1))+1;    
    fullarray = new node_idx[psize];

    for (i = 0; i < psize; i++)
	fullarray[i] = 0;
    for (i = 0; i < nodeP->size; i++) {
	int idx = SPARSE_INDEX(k, nodeP, i);

	assert(idx < psize);
	fullarray[idx] = SPARSE_ARC(k, nodeP, i);
    }
    return psize;
}

DynArray < int *>*states;
int counter;
int *tempStates;

void Forest::PrintStates(node_idx root)
{
    states = new DynArray < int *>;

    counter = 0;
    tempStates = new int[K + 1];

    PrintStates(K, root, tempStates);
    printf("%d reachable states.  Here they are: \n", counter);
    for (int i = 0; i < counter; i++) {
	printf("(");
	for (level k = K; k > 0; k--) {
	    if ((*(*states)[i])[k] == -1)
		printf("* ");
	    else
		printf("%d ", (*(*states)[i])[k]);
	}
	printf(")\n");
    }
}

void Forest::PrintVals(MDDHandle root, level k)
{
    for (level k1 = K; k1 > 0; k1--) {
	FlushCaches(k1);
	for (int i = 0; i < last[k1]; i++) {
	    Node *child;

	    child = &FDDL_NODE(k1, i);
	    if (child->flags & FLAG_SHARED)
		child->flags = (flag)(child->flags - FLAG_SHARED);
	}
    }
    InternalPrintVals(k, root.index);	//Mark all nodes in Query
    for (int i = 0; i < last[k]; i++) {
	Node *nodeP = &FDDL_NODE(k, i);

	if (nodeP->flags & FLAG_SHARED) {
	    for (int j = 0; j < nodeP->size; j++) {
		if (FDDL_NODE(k - 1, FDDL_ARC(k, nodeP, j)).flags & FLAG_SHARED)
		    printf("%d ", j);
	    }
	}
    }
    printf("\n");
}

void Forest::PrintRanges(MDDHandle root, level * mask)
{
    node_idx newRoot;
    PrintNode *p;
    PrintNode *stack;
    int *low;
    int *high;
    low = new int[K + 1];
    high = new int[K + 1];
    newRoot = Projection(K, root.index, mask);	//Remove extraneous data.
    stack = NULL;

    for (level k = K; k >= 0; k--)
	low[k] = high[k] = 0;

    PrintRanges(K, newRoot, mask, stack, low, high);
    delete[]high;
    delete[]low;
    p = stack;
    while (p != NULL) {
	p->Print(mask);
	stack = p;
	p = p->next;
	delete stack;
    }
    printf("\n");
}

void Forest::PrintAddy(MDDHandle root, level k)
{
    node_idx newRoot;
    level mask[K + 1];

    for (int i = 0; i <= K; i++)
	mask[k] = 0;

    for (int i = 0; i < 4; i++) {
	mask[k - i] = 1;
    }

    newRoot = Projection(K, root.index, mask);

    //newRoot = ProjectVals(K, root.index, k - 3);

    for (level k1 = K; k1 > 0; k1--) {
	FlushCaches(k1);
	for (int i = 0; i < last[k1]; i++) {
	    Node *child;

	    child = &FDDL_NODE(k1, i);
	    if (child->flags & FLAG_SHARED)
		child->flags = (flag)(child->flags - FLAG_SHARED);
	}
    }

    InternalPrintVals(K, newRoot);	//Mark all nodes in Query
    numSolutions = 0;
    int *vals = new int[4];

    for (int i = 0; i < 4; i++)
	vals[i] = (-1);
    for (int i = 0; i < last[k]; i++) {
	if (FDDL_NODE(k, i).flags & FLAG_SHARED)
	    PrintAddy(k, i, vals, 0);
    }
    delete[]vals;
    printf("\n# %.0f result%s\n", numSolutions,
	   numSolutions == 1 ? "." : "s.");
}

void Forest::PrintRanges(level k, node_idx p, level * mask,
			 PrintNode * &stack, int *low, int *high)
{
    node_idx child;
    Node *nodeP;
    node_idx iVal;
    node_idx lastVal;

    PrintNode *pn;

    if (p == 0)
	return;

    if (k == 0) {
	PrintNode *newNode;
	newNode = new PrintNode(K, maxVals);
	newNode->next = stack;
	for (level k1 = K; k1 > 0; k1--) {
	    newNode->low[k1] = low[k1];
	    newNode->high[k1] = high[k1];
	}
	stack = newNode;
	return;
    }

    nodeP = &FDDL_NODE(k, p);
    if (mask[k] == 1) {
	arc_idx cur_low;
	lastVal = FDDL_ARC(k, nodeP, 0);
	cur_low = 0;
	for (arc_idx i = 1; i < nodeP->size; i++) {
	    iVal = FDDL_ARC(k, nodeP, i);
	    if (iVal != lastVal) {
		low[k] = cur_low;
		high[k] = i - 1;
		PrintRanges(k - 1, lastVal, mask, stack, low, high);
		cur_low = i;
		lastVal = iVal;
	    }
	}
	low[k] = cur_low;
	high[k] = nodeP->size - 1;
	PrintRanges(k - 1, lastVal, mask, stack, low, high);
    } else {
	child = FDDL_ARC(k, nodeP, 0);
	low[k] = 0;
	high[k] = maxVals[k];
	PrintRanges(k - 1, child, mask, stack, low, high);
    }
}

void Forest::PrintAddy(level k, node_idx p, int *vals, int depth)
{
    int i;
    int lastVal;

    if (p == 0)
	return;

    Node *nodeP = &FDDL_NODE(k, p);

    if (!(nodeP->flags & FLAG_SHARED))
	return;

    float numResults;

    if (depth > 3) {
	numResults = 1;
	for (i = 0; i < 4; i++) {
	    if (vals[i] >= 0)
		printf("%d%c", vals[i], i == 3 ? ' ' : '.');
	    else {
		printf("*%c", i == 3 ? ' ' : '.');
		numResults *= 256;
	    }
	}
	numSolutions += numResults;
	printf("\n");
	return;
    }
    if (nodeP->size == maxVals[k] + 1) {
	i = 0;
	lastVal = FDDL_ARC(k, nodeP, i);
	for (i = 1; i < nodeP->size; i++) {
	    if (FDDL_ARC(k, nodeP, i) != lastVal)
		break;
	}
	if (i == nodeP->size) {
	    vals[depth] = (-1);
	    PrintAddy(k - 1, FDDL_ARC(k, nodeP, 0), vals, depth + 1);	//The last one, wlog.
	    return;
	}
    }
    for (i = 0; i < nodeP->size; i++) {
	int j = FDDL_ARC(k, nodeP, i);

	vals[depth] = i;
	PrintAddy(k - 1, j, vals, depth + 1);
    }
}

int Forest::InternalPrintVals(level k, node_idx p)
{
    int result;

    if (p == 0) {
	return 0;
    }
    if (k == 0) {
	if (p == 1) {
	    return 1;
	}
	return 0;
    }
    result = PrintCache[k]->hit(p);
    if (result >= 0) {
	return result;
    }

    Node *nodeP = &FDDL_NODE(k, p);

    result = 0;
    for (int i = 0; i < nodeP->size; i++) {
	if (InternalPrintVals(k - 1, FDDL_ARC(k, nodeP, i)) == 1) {
	    result = 1;
	    //break;
	}
    }
    if (result == 1) {
	nodeP->flags = (flag)(nodeP->flags | FLAG_SHARED);
	PrintCache[k]->add(1, p);
	return 1;
    }
    PrintCache[k]->add(0, k, p);
    return 0;
}

node_idx Forest::ProjectVals(level k, node_idx p, level cutoff)
{
    Node *nodeP;
    node_idx result;
    node_idx flag;

    //Check Base Cases
    if (p == 0)
	return 0;

    if (k == 0)
	return 1;

    //Check Cache
    result = ProjectCache[k]->hit(p);
    if (result >= 0) {
	return result;
    }

    nodeP = &FDDL_NODE(k, p);

    if (k < cutoff) {
	flag = 0;
	for (node_idx i = 0; i < nodeP->size; i++) {
	    flag = ProjectVals(k - 1, FDDL_ARC(k, nodeP, i), cutoff);
	    if (flag != 0)
		break;
	}
	if (flag != 0) {
	    result = NewNode(k);
	    for (node_idx i = 0; i <= maxVals[k]; i++) {
		SetArc(k, result, i, flag);
	    }
	    result = CheckIn(k, result);
	    ProjectCache[k]->add(result, p);
	    return result;
	}
    } else {
	result = NewNode(k);
	for (node_idx i = 0; i < nodeP->size; i++) {
	    SetArc(k, result, i,
		   ProjectVals(k - 1, FDDL_ARC(k, nodeP, i), cutoff));
	}
	result = CheckIn(k, result);
	ProjectCache[k]->add(result, p);
	return result;
    }
    return 0;
}

node_idx Forest::Projection(level k, node_idx p, level * mask)
{
    Node *nodeP;
    node_idx result;
    node_idx flag;
    node_idx u;

    //Check Base Cases
    if (p == 0)
	return 0;

    if (k == 0)
	return 1;

    //Check Cache
    result = ProjectCache[k]->hit(p);
    if (result >= 0) {
	return result;
    }

    nodeP = &FDDL_NODE(k, p);

    if (mask[k] == 0) {
	flag = 0;
	for (node_idx i = 0; i < nodeP->size; i++) {
	    u = Projection(k - 1, FDDL_ARC(k, nodeP, i), mask);
	    flag = InternalMax(k - 1, u, flag);
	}
	if (flag != 0) {
	    result = NewNode(k);
	    for (node_idx i = 0; i <= maxVals[k]; i++) {
		SetArc(k, result, i, flag);
	    }
	    result = CheckIn(k, result);
	    ProjectCache[k]->add(result, p);
	    return result;
	}
    } else {
	result = NewNode(k);
	for (node_idx i = 0; i < nodeP->size; i++) {
	    SetArc(k, result, i,
		   Projection(k - 1, FDDL_ARC(k, nodeP, i), mask));
	}
	result = CheckIn(k, result);
	ProjectCache[k]->add(result, p);
	return result;
    }
    return 0;
}

void Forest::PrintStates(level k, node_idx p, int *stateArray)
{
    arc_idx i;
    Node *nodeP;

    if (p == 0)
	return;
    else if (k == 0) {
	stateArray[k] = p;
	(*(*states)[counter]) = new int[K + 1];

	for (level k = K; k >= 0; k--)
	    (*(*states)[counter])[k] = stateArray[k];
	counter++;
    } else {
	nodeP = &FDDL_NODE(k, p);
	if (nodeP->flags & FLAG_SPARSE) {
	    for (i = 0; i < nodeP->size; i++) {
		stateArray[k] = FULL_ARC(k, nodeP, i * 2);
		PrintStates(k - 1,
				FULL_ARC(k, nodeP, i * 2 + 1),
				stateArray);
	    }
	} else {
	    for (i = 0; i < nodeP->size; i++) {
		int arc;

		if (FDDL_ARC(k, nodeP, i) != arc) {
		    break;
		}
		arc = FDDL_ARC(k, nodeP, i);
	    }
	    if (i == nodeP->size) {
		stateArray[k] = (-1);
		PrintStates(k - 1, FDDL_ARC(k, nodeP, i), stateArray);
	    } else {
		for (i = 0; i < nodeP->size; i++) {
		    stateArray[k] = i;
		    PrintStates(k - 1, FULL_ARC(k, nodeP, i), stateArray);
		}
	    }
	}
    }
}

void Forest::PruneMDD(MDDHandle p)
{

/*
for (node_idx i=0; i< last[K];i++)
      if (i != p.index){
            DeleteNode(K,i);
            (*nodes[K])[i].in = 0;
         }
*/
    for (level k = K; k > 0; k--)
	PruneCache[k]->clear();
    InternalPruneMDD(K, p.index, 1);
    for (level k = K; k > 0; k--) {
	for (node_idx q = 1; q < last[k]; q++) {
	    Node *nodeQ;

	    nodeQ = &FDDL_NODE(k, q);
	    if (!(nodeQ->flags & FLAG_SHARED)) {
		DeleteDownstream(k, q);
	    }
	}
    }
    for (level k = K; k > 0; k--)
	PruneCache[k]->clear();
    InternalPruneMDD(K, p.index, 1);
}

void Forest::PruneMDD(node_idx p)
{
    for (level k = K; k > 0; k--)
	PruneCache[k]->clear();
    InternalPruneMDD(K, p, 1);
    for (level k = K; k > 0; k--) {
	for (node_idx q = 1; q < last[k]; q++) {
	    Node *nodeQ;

	    nodeQ = &FDDL_NODE(k, q);
	    if (!(nodeQ->flags & FLAG_SHARED)) {
		DeleteDownstream(k, q);
	    }
	}
    }
    for (level k = K; k > 0; k--)
	PruneCache[k]->clear();
    InternalPruneMDD(K, p, 1);
}

void Forest::InternalPruneMDD(level k, node_idx p, int shared_flag)
{
    Node *nodeP;
    int result;

    if (k == 0 || p == 0)
	return;
    nodeP = &FDDL_NODE(k, p);
    result = PruneCache[k]->hit(p);
    if (result == 1)
	return;
    if (shared_flag == 0) {
	if (nodeP->flags & FLAG_SHARED)
	    nodeP->flags = (flag)(nodeP->flags - FLAG_SHARED);
    } else {
	if (nodeP->flags & FLAG_SHARED)
	    return;
	nodeP->flags = (flag)(nodeP->flags | FLAG_SHARED);
    }
    for (int i = 0; i < nodeP->size; i++) {
	InternalPruneMDD(k - 1, FDDL_ARC(k, nodeP, i), shared_flag);
    }
    PruneCache[k]->add(1, p);
}

void Forest::PrintMDD()
{
    int count;
    int lastVal;

    count = 0;
    for (level k = K; k >= 1; k--) {
	printf("Level %d: ", k);
	for (int i = 1; i < last[k]; i++) {
	    Node *nodeI;

	    nodeI = &FDDL_NODE(k, i);
	    if (!(nodeI->flags & FLAG_DELETED)) {
		printf("%d:%c(%d,%d)%c ", i,
		       nodeI->flags & FLAG_SHARED ? '!' : ' ',
		       nodeI->down,
		       nodeI->size, nodeI->flags & FLAG_SPARSE ? 'S' : ' ');
	    } else {
		printf("%d[D] ", i);
	    }
	}
	printf("\t: %d\n", tail[k]);
	lastVal = (-1);
	count = 0;
	for (int i = 0; i < tail[k]; i++) {
	    if ((*(*arcs[k])[i]) == lastVal) {
		count++;
	    } else {
		if (count != 0)
		    printf("[%d] ", count + 1);
		else
		    printf(" ");
		printf("%d", (*(*arcs[k])[i]));
		count = 0;
	    }
	    lastVal = (*(*arcs[k])[i]);
	}
	if (count != 0)
	    printf("[%d] ", count + 1);
	printf("\n");
    }
}

void Forest::PrintMDD(int top, int bottom)
{
    for (level k = top; k >= bottom; k--) {
	printf("Level %d: ", k);
	for (int i = 1; i < last[k]; i++)
	    if (!(FDDL_NODE(k, i).flags & FLAG_DELETED))
		printf("%d:(%d,%d)%c ", i, FDDL_NODE(k, i).down,
		       FDDL_NODE(k, i).size, FDDL_NODE(k,
						       i).flags & FLAG_SPARSE ?
		       'S' : ' ');
	printf("\t: %d\n", tail[k]);
	for (int i = 0; i < tail[k]; i++)
	    printf("%d ", (*(*arcs[k])[i]));
	printf("\n");
    }
}

void Forest::SaveMDD(char *filename)
{
    FILE *outFile = fopen(filename, "w");

    fprintf(outFile, "%d\n", K);	//Number of Levels

    for (level k = K; k > 0; k--) {
	fprintf(outFile, "%d %d\n", last[k], tail[k]);	//Last and Tail
    }

    for (level k = K; k > 0; k--) {
	fprintf(outFile, "%d\n", maxVals[k]);	//Maxvals
    }

    for (level k = K; k > 0; k--) {	//Nodes
	for (node_idx i = 0; i < last[k]; i++) {
	    Node *nodeI = &FDDL_NODE(k, i);

	    fprintf(outFile, "%d %d %d %d ", nodeI->flags,
		nodeI->in, nodeI->size, nodeI->down);
	}
	fprintf(outFile, "\n");
	for (arc_idx j = 0; j < tail[k]; j++) {	//Arcs
	    fprintf(outFile, "%d ", (*(*arcs[k])[j]));
	}
	fprintf(outFile, "\n");
    }
    fclose(outFile);
}

void Forest::LoadMDD(char *filename)
{
    FILE *inFile = fopen(filename, "r");

    fscanf(inFile, "%d\n", &K);
    maxVals = new int[K + 1];

    nodes = new DynArray < Node > *[K + 1];
    arcs = new DynArray < node_idx > *[K + 1];

    for (int k = 1; k <= K; k++) {
	nodes[k] = new DynArray < Node >;
	arcs[k] = new DynArray < node_idx >;
    }
    last = new node_idx[K + 1];
    tail = new int[K + 1];

    for (level k = K; k > 0; k--) {
	fscanf(inFile, "%d %d\n", &last[k], &tail[k]);
    }
    for (level k = K; k > 0; k--) {
	fscanf(inFile, "%d\n", &maxVals[k]);
    }
    for (level k = K; k > 0; k--) {
	for (node_idx i = 0; i < last[k]; i++) {
	    Node *nodeI = &FDDL_NODE(k, i);

	    fscanf(inFile, "%d %d %d %d ", &nodeI->flags,
		&nodeI->in, &nodeI->size, &nodeI->down);
	}
	fscanf(inFile, "\n");
	for (arc_idx j = 0; j < tail[k]; j++) {
	    fscanf(inFile, "%d ", (*arcs[k])[j]);
	}
	fscanf(inFile, "\n");
    }
}
