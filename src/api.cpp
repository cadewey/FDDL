/*
 * FDDL : The Free Decision Diagram Library 
 * Copyright (C) 2004 Robert Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

#include <climits>
#include <stdio.h>
#include <assert.h>
#include "mdd.h"

#define MAX(a, b) (a>b ? a : b)
#define MIN(a, b) (a<b ? a : b)

#define NON_DEBUG 1

int Forest::MakeMDDFromTuple(int *low, int *high, MDDHandle & ref)
{
    node_idx child, top;
    arc_idx s;
    level k;

#ifdef BRIEF_DEBUG
    printf("MakeMDDFromTuple:\n");
    printf("\t");
    for (k = K; k >= 0; k--) {
	printf("%d-%d ", low[k], high[k]);
    }
    printf("\n");
#endif

    child = high[0];
    if (child > maxVals[0]) {
	printf("%d out of range at level %d\n", high[k], k);
	assert(0);
	return TUPLE_OUT_OF_BOUNDS;
    }
    for (k = 1; k <= K; k++) {
	if (high[k] > maxVals[k]) {
	    DeleteNode(k - 1, child);
	    printf("%d out of range at level %d\n", high[k], k);
	    assert(0);
	    return TUPLE_OUT_OF_BOUNDS;
	}
	top = NewNode(k);
	for (s = low[k]; s <= high[k]; s++) {
	    SetArc(k, top, s, child);
	}
	child = CheckIn(k, top);
    }
    if (ref.index != child) {
	ReallocHandle(ref);
	Attach(ref, child);
    }
#ifdef BRIEF_DEBUG
    printf("MakeMDDFromTuple:\n");
    printf("\tIndex: %d\n", ref.index);
#endif
    return SUCCESS;
}

int Forest::Assign(MDDHandle root, int *low, int *high, MDDHandle & result)
{
    level k;
    node_idx child, newNode;
    arc_idx s;

#ifdef BRIEF_DEBUG
    printf("MDD Assign: %d\n", root.index);
    printf("\t");
    for (k = K; k >= 0; k--) {
	printf("%d-%d ", low[k], high[k]);
    }
    printf("\n");
#endif

    child = high[0];
    if (child > maxVals[0]) {
	printf("%d out of range at level %d\n", child, k);
	assert(0);
	return TUPLE_OUT_OF_BOUNDS;
    }

    if (root.index < 0) {
	return MakeMDDFromTuple(low, high, result);
    }
    for (k = 1; k <= K; k++) {
	if (high[k] > maxVals[k]) {
	    if (FDDL_NODE(k - 1, child).in == 1)
		DeleteDownstream(k - 1, child);
	    printf("%d out of range at level %d\n", high[k], k);
	    assert(0);
	    return TUPLE_OUT_OF_BOUNDS;
	}
	newNode = NewNode(k);
	for (s = low[k]; s <= high[k]; s++) {
	    SetArc(k, newNode, s, child);
	}
	newNode = CheckIn(k, newNode);
	child = newNode;
    }
    newNode = InternalRestrict(K, root.index, child);
    if (newNode != child)
	DeleteDownstream(K, child);
    if (result.index != newNode) {
	ReallocHandle(result);
	Attach(result, newNode);
    }
#ifdef BRIEF_DEBUG
    printf("Assign Result: %d\n", result.index);
#endif
    return SUCCESS;
}

int Forest::LessThan(MDDHandle root, int value, MDDHandle & result)
{
    if (root.index < 0)
	return INVALID_MDD;
    node_idx newresult;

    for (level k = K; k > 0; k--)
	LessThanCache[k]->clear();
    newresult = InternalLessThan(K, root.index, value);
    if (result.index != newresult) {
	ReallocHandle(result);
	Attach(result, newresult);
    }
    return SUCCESS;
}

int Forest::Apply(MDDHandle * roots, int num_roots,
		  node_idx(*func) (node_idx *, int), MDDHandle & result)
{

    node_idx *indices;
    node_idx newresult;
    if (num_roots < 1)
	return INVALID_MDD;

    indices = new node_idx[num_roots];

    for (int i = 0; i < num_roots; i++) {
	indices[i] = roots[i].index;
	if (roots[i].index < 0) {
	    delete[]indices;
	    return INVALID_MDD;
	}
    }

    for (level k = K; k > 0; k--)
	ApplyCache[k]->clear();

    newresult = InternalApply(K, indices, num_roots, func);
    if (result.index != newresult) {
	ReallocHandle(result);
	Attach(result, newresult);
    }
    delete[]indices;
    return SUCCESS;
}

int Forest::ValRestrict(MDDHandle root, int value, MDDHandle & result)
{
    if (root.index < 0)
	return INVALID_MDD;
    node_idx newresult;

    newresult = InternalValRestrict(K, root.index, value);
    if (result.index != newresult) {
	ReallocHandle(result);
	Attach(result, newresult);
    }
    return SUCCESS;
}

int Forest::Select(MDDHandle root, int num_chains,
		   MDDHandle * all_roots, MDDHandle & result)
{
    node_idx *child_array;
    node_idx newresult;

    child_array = new node_idx[num_chains];

    if (root.index < 0)
	return INVALID_MDD;
    for (int i = 0; i < num_chains; i++) {
	child_array[i] = all_roots[i].index;
	if (child_array[i] < 0) {
	    delete[]child_array;
	    return INVALID_MDD;
	}
    }
    newresult = InternalSelect(K, root.index, num_chains, child_array);
    if (result.index != newresult) {
	ReallocHandle(result);
	Attach(result, newresult);
    }
    return SUCCESS;
}

int Forest::Replace(MDDHandle p, MDDHandle q, bool strict,
		    MDDHandle & result)
{

    node_idx newresult;

    for (level k = K; k > 0; k--) {
	ReplaceCache[k]->clear();
	ReplaceStrictCache[k]->clear();
    }
    if (p.index < 0)
	return INVALID_MDD;
    if (q.index < 0)
	return INVALID_MDD;

    if (strict) {
	newresult = InternalReplaceStrict(K, p.index, q.index);
    } else {
	newresult = InternalReplace(K, p.index, q.index);
    }
    if (result.index != newresult) {
	ReallocHandle(result);
	Attach(result, newresult);
    }
    return SUCCESS;
}

int Forest::ProjectOnto(MDDHandle p, MDDHandle q, MDDHandle & result)
{

    node_idx newresult;

    for (level k = K; k > 0; k--) {
	ProjectOntoCache[k]->clear();
    }
    if (p.index < 0)
	return INVALID_MDD;

    if (q.index < 0)
	return INVALID_MDD;

    newresult = InternalProjectOnto(K, p.index, q.index);

    if (result.index != newresult) {
	ReallocHandle(result);
	Attach(result, newresult);
    }
    return SUCCESS;
}

int Forest::Combine(MDDHandle root, MDDHandle root2,
		    int chain_index, MDDHandle & result)
{
    if (root.index < 0)
	return INVALID_MDD;
    if (root2.index < 0)
	return INVALID_MDD;
    node_idx newresult;

    newresult = InternalCombine(K, root.index, root2.index, chain_index);
    if (result.index != newresult) {
	ReallocHandle(result);
	Attach(result, newresult);
    }
    return SUCCESS;
}

node_idx Forest::InternalLessThan(level k, node_idx p, int value)
{
    arc_idx i;
    node_idx result, u;
    Node *victim;
    int psize;

    if (k == 0) {
	if (p < value)
	    return p;
	return 0;
    };

    if (p == 0) {
	return 0;
    }

    result = LessThanCache[k]->hit(p, value);
    if (result >= 0)
	return result;

    result = NewNode(k);
    victim = &FDDL_NODE(k, p);
    psize = victim->size;
    for (i = 0; i < psize; i++) {
	u = InternalLessThan(k - 1, FULL_ARC(k, victim, i), value);
	SetArc(k, result, i, u);
    }
    result = CheckIn(k, result);
    LessThanCache[k]->add(result, p, value);
    return result;
}

node_idx
    Forest::InternalApply(level k, node_idx * roots, int num_roots,
			  node_idx(*func) (node_idx *, int))
{
    node_idx i;
    arc_idx j;
    node_idx result, u;
    node_idx *indices;

    if (k == 0) {
	return func(roots, num_roots);
    };

    result = ApplyCache[k]->hit(roots, num_roots);
    if (result >= 0)
	return result;

    result = NewNode(k);

    indices = new node_idx[num_roots];

    for (j = 0; j <= maxVals[k]; j++) {
	for (i = 0; i < num_roots; i++) {
	    Node *nodeP;
	    nodeP = &FDDL_NODE(k, roots[i]);
	    if (j < nodeP->size) {
		indices[i] = FDDL_ARC(k, nodeP, j);
	    } else
		indices[i] = 0;
	}
	u = InternalApply(k - 1, indices, num_roots, func);
	SetArc(k, result, j, u);
    }
    result = CheckIn(k, result);
    ApplyCache[k]->add(result, roots, num_roots);
    delete[]indices;
    return result;
}

node_idx Forest::InternalValRestrict(level k, node_idx p, int value)
{
    arc_idx i;
    node_idx result, u;
    Node *victim;
    int psize;
    arc_idx *ptemp;

    if (k == 0) {
	if (p == value)
	    return p;
	return 0;
    };

    result = ValRestrictCache[k]->hit(p, value);
    if (result >= 0)
	return result;

    result = NewNode(k);
    if (p == 0) {
	return 0;
    }
    victim = &FDDL_NODE(k, p);
    if (IS_SPARSE(victim)) {	//If node <k.p> is stored sparsely, unpack it into a static array of appropriate size
	psize = UnpackNode(k, p, ptemp);
    } else {
	psize = victim->size;
	ptemp = new node_idx[psize];

	for (i = 0; i < psize; i++)
	    ptemp[i] = FULL_ARC(k, victim, i);
    }
    for (i = 0; i < psize; i++) {
	u = InternalValRestrict(k - 1, ptemp[i], value);
	SetArc(k, result, i, u);
    }
    result = CheckIn(k, result);
    ValRestrictCache[k]->add(result, p, value);
    delete[]ptemp;
    return result;
}

node_idx
    Forest::InternalSelect(level k, node_idx p, int num_chains,
			   node_idx * child_array)
{

    if (p == 0) {
	return p;
    }

    if (k == 0) {
	while (p > 4) {
	    assert(p - 4 < num_chains);
	    p = child_array[p - 4];
	}
	return p;
    }

    node_idx r;

    r = SelectCache[k]->hit(p, child_array, num_chains);
    if (r >= 0)
	return r;

    Node *nodeP;
    Node *nodeR;

    r = NewNode(k);
    nodeP = &FDDL_NODE(k, p);
    nodeR = &FDDL_NODE(k, r);

    for (int i = 0; i < nodeP->size; i++) {
	node_idx u;
	node_idx *grandchild_array;
	grandchild_array = new node_idx[num_chains];

	for (int j = 0; j < num_chains; j++) {
	    Node *nodeJ;

	    nodeJ = &FDDL_NODE(k, child_array[j]);
	    grandchild_array[j] = FDDL_ARC(k, nodeJ, i);
	}
	u = InternalSelect(k - 1, FDDL_ARC(k, nodeP, i), num_chains,
			   grandchild_array);
	delete[]grandchild_array;
	//Do I need to union what's already there with what's coming?
	SetArc(k, r, i, u);
	//No.
    }
    r = CheckIn(k, r);
    SelectCache[k]->add(r, p, child_array, num_chains);
    return r;
}

node_idx Forest::InternalReplace(level k, node_idx p, node_idx q)
{

    if (p == 0 || p == q)
	return q;

    if (q == 0) {
	return p;
    }

    if (k == 0) {
	/*
	 * if (p == 0)
	 * return 0;
	 * if (q == 0)
	 * return p;
	 */
	return q;
    }

    node_idx r;

    r = ReplaceCache[k]->hit(p, q);
    if (r >= 0)
	return r;

    Node *nodeP;
    Node *nodeQ;
    Node *nodeR;

    nodeP = &FDDL_NODE(k, p);
    nodeQ = &FDDL_NODE(k, q);

    r = NewNode(k);
    nodeR = &FDDL_NODE(k, r);

    for (int i = 0; i < MAX(nodeP->size, nodeQ->size); i++) {
	node_idx u;

	u = InternalReplace(k - 1,
			    i < nodeP->size ? FDDL_ARC(k, nodeP, i) : 0,
			    i < nodeQ->size ? FDDL_ARC(k, nodeQ, i) : 0);
	SetArc(k, r, i, u);
    }
    r = CheckIn(k, r);
    ReplaceCache[k]->add(r, p, q);
    return r;
}

node_idx Forest::InternalProjectOnto(level k, node_idx p, node_idx q)
{

    if (q == 0)
	return 0;

    if (p == 0) {
	return q;
    }

    if (k == 0)
	return p;

    node_idx r;

    r = ProjectOntoCache[k]->hit(p, q);
    if (r >= 0)
	return r;

    Node *nodeP;
    Node *nodeQ;
    Node *nodeR;

    nodeP = &FDDL_NODE(k, p);
    nodeQ = &FDDL_NODE(k, q);

    r = NewNode(k);
    nodeR = &FDDL_NODE(k, r);

    for (int i = 0; i < MAX(nodeP->size, nodeQ->size); i++) {
	node_idx u;

	u = InternalProjectOnto(k - 1, i < nodeP->size ?
				FDDL_ARC(k, nodeP, i) : 0,
				i < nodeQ->size ? FDDL_ARC(k, nodeQ,
							   i) : 0);

	SetArc(k, r, i, u);
    }
    r = CheckIn(k, r);
    ProjectOntoCache[k]->add(r, p, q);
    return r;
}

node_idx Forest::InternalReplaceStrict(level k, node_idx p, node_idx q)
{
    if (p == 0) {
	return 0;
    }

    if (q == 0) {
	return p;
    }

    if (k == 0) {
	if (p == 0)
	    return 0;
	if (q == 0)
	    return p;
	return q;
    }

    node_idx r;

    r = ReplaceStrictCache[k]->hit(p, q);
    if (r >= 0) {
	return r;
    }

    Node *nodeP;
    Node *nodeQ;
    Node *nodeR;

    nodeP = &FDDL_NODE(k, p);
    nodeQ = &FDDL_NODE(k, q);

    r = NewNode(k);
    nodeR = &FDDL_NODE(k, r);

    for (int i = 0; i < MAX(nodeP->size, nodeQ->size); i++) {
	node_idx u;

	u = InternalReplaceStrict(k - 1,
				  i < nodeP->size ? FDDL_ARC(k, nodeP,
							     i) : 0,
				  i < nodeQ->size ? FDDL_ARC(k, nodeQ,
							     i) : 0);
	SetArc(k, r, i, u);
    }
    r = CheckIn(k, r);
    ReplaceStrictCache[k]->add(r, p, q);
    return r;
}

node_idx
    Forest::InternalCombine(level k, node_idx p, node_idx q,
			    int chain_index)
{
    arc_idx i;
    node_idx result, u;
    Node *nodeP, *nodeQ;
    int psize, qsize;
    int dummy;
    arc_idx *ptemp;
    arc_idx *qtemp;

    if (p == 0)
	return 0;
    if (q == 0)
	return p;
    if (k == 0) {
	if (p == chain_index + 4) {
	    return q;
	}
	return p;
    }
    dummy = 1;
    result = CombineCache[k]->hit(p, q, chain_index);
    if (result >= 0)
	return result;

    result = NewNode(k);
    nodeP = &FDDL_NODE(k, p);
    nodeQ = &FDDL_NODE(k, q);

    if (IS_SPARSE(nodeP)) {	//If node <k.p> is stored sparsely, unpack it into a static array of appropriate size
	psize = UnpackNode(k, p, ptemp);
    } else {
	psize = nodeP->size;
	ptemp = new node_idx[psize];

	for (i = 0; i < psize; i++)
	    ptemp[i] = FULL_ARC(k, nodeP, i);
    }
    if (IS_SPARSE(nodeQ)) {	//If node <k.q> is stored sparsely, unpack it into a static array of appropriate size
	qsize = UnpackNode(k, q, qtemp);
    } else {
	qsize = nodeQ->size;
	qtemp = new node_idx[qsize];

	for (i = 0; i < qsize; i++)
	    qtemp[i] = FULL_ARC(k, nodeQ, i);
    }
    for (i = 0; i < psize; i++) {
	u = InternalCombine(k - 1, ptemp[i], i < qsize ? qtemp[i] : 0,
			    chain_index);
	SetArc(k, result, i, u);
    }
    delete[]qtemp;
    delete[]ptemp;
    result = CheckIn(k, result);
    CombineCache[k]->add(result, p, q, chain_index);
    return result;
}

int
 Forest::Max(MDDHandle a, MDDHandle b, MDDHandle & result)
{
    if (a.index < 0 || b.index < 0)
	return MAX_FAILED;
    node_idx newresult;

    newresult = InternalMax(K, a.index, b.index);
    if (result.index != newresult) {
	ReallocHandle(result);
	Attach(result, newresult);
    }
    return SUCCESS;
}

int Forest::Min(MDDHandle a, MDDHandle b, MDDHandle & result)
{
    if (a.index < 0 || b.index < 0)
	return MIN_FAILED;
    node_idx newresult;

    newresult = InternalMin(K, a.index, b.index);
    if (result.index != newresult) {
	ReallocHandle(result);
	Attach(result, newresult);
    }
    return SUCCESS;
}

int Forest::Complement(MDDHandle a, MDDHandle & result)
{
    if (a.index < 0)
	return COMPLEMENT_FAILED;
    node_idx newresult;

    newresult = InternalComplement(K, a.index);
    if (result.index != newresult) {
	ReallocHandle(result);
	Attach(result, newresult);
    }
    return SUCCESS;
}

int Forest::BinaryComplement(MDDHandle a, MDDHandle & result)
{
    if (a.index < 0)
	return COMPLEMENT_FAILED;
    node_idx newresult;

    newresult = InternalBComplement(K, a.index);
    if (result.index != newresult) {
	ReallocHandle(result);
	Attach(result, newresult);
    }
    return SUCCESS;
}

//Simple Recursive Minimum of <k,p> and <k,q>

node_idx Forest::InternalMin(level k, node_idx p, node_idx q)
{
    //Easy Terminal Cases
    if (p == 0 || q == 0)
	return 0;
    if (p == q)
	return q;
    if (k == 0)
	return p > q ? q : p;

    //Check for an entry in the Cache.
    node_idx result;

    result = MinCache[k]->hit(p, q);
    if (result >= 0) {
	if (!(FDDL_NODE(k, result).flags & DELETED))
	    return result;
	return CheckIn(k, result);
    }

    result = NewNode(k);
    Node *nodeP = &FDDL_NODE(k, p);
    Node *nodeQ = &FDDL_NODE(k, q);

    int psize = nodeP->size;
    int qsize = nodeQ->size;

    //If neither node is sparse, do things the easy way.
    if (!IS_SPARSE(nodeP) && !IS_SPARSE(nodeQ)) {
	for (arc_idx i = 0; i < (psize > qsize ? psize : qsize); i++) {
	    node_idx u = InternalMin(k - 1,
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

	for (arc_idx i = 0, j = 0; i < psize && j < qsize;) {
	    arc_idx pdx = SPARSE_INDEX(k, nodeP, i);
	    node_idx pval = SPARSE_ARC(k, nodeP, i);
	    arc_idx qdx = SPARSE_INDEX(k, nodeQ, j);
	    node_idx qval = SPARSE_ARC(k, nodeQ, j);

	    if (pdx < qdx) {
		SetArc(k, result, pdx, 0);
		i++;
	    } else if (qdx < pdx) {
		SetArc(k, result, qdx, 0);
		j++;
	    } else {
		SetArc(k, result, pdx, InternalMin(k - 1, pval, qval));
		i++;
		j++;
	    }
	}
    } else {
	if (IS_SPARSE(nodeP) && !IS_SPARSE(nodeQ)) {
	    int j = 0;

	    for (int i = 0; i < nodeP->size && j < nodeQ->size;) {
		int idx = SPARSE_INDEX(k, nodeP, i);
		int ival = SPARSE_ARC(k, nodeP, i);
		int jval = FULL_ARC(k, nodeQ, j);

		if (j < idx) {
		    SetArc(k, result, j, 0);
		    j++;
		} else if (idx < j) {
		    SetArc(k, result, idx, 0);
		    i++;
		} else {
		    SetArc(k, result, j, InternalMin(k - 1, ival, jval));
		    i++;
		    j++;
		}
	    }
	} else if (IS_SPARSE(nodeQ) && !IS_SPARSE(nodeP)) {
	    int i = 0;

	    for (int j = 0; j < nodeQ->size && i < nodeP->size;) {
		int jdx = SPARSE_INDEX(k, nodeQ, j);
		int jval = SPARSE_ARC(k, nodeQ, j);
		int ival = FULL_ARC(k, nodeP, i);

		if (i < jdx) {
		    SetArc(k, result, i, 0);
		    i++;
		} else if (jdx < i) {
		    SetArc(k, result, jdx, 0);
		    j++;
		} else {
		    SetArc(k, result, i, InternalMin(k - 1, ival, jval));
		    i++;
		    j++;
		}
	    }

	}
    }

    node_idx newresult = CheckIn(k, result);

//      if (k > 0 && newresult)
//              FDDL_NODE(k, newresult).flags |= CHECKED_IN;
    MinCache[k]->add(newresult, p, q);
    MinCache[k]->add(newresult, q, p);
    MinCache[k]->add(newresult, p, newresult);
    MinCache[k]->add(newresult, q, newresult);
    return newresult;
}

//Simple Recursive Complement of <k,p> and <k,q>

node_idx Forest::InternalComplement(level k, node_idx p)
{
    //Easy Terminal Cases
    if (k == 0) {
	return maxVals[0] - p;
    }
    //Check for an entry in the Cache.
    node_idx result;

    result = ComplementCache[k]->hit(p);
    if (result >= 0) {
	if (!(FDDL_NODE(k, result).flags & DELETED))
	    return result;
	return CheckIn(k, result);
    }

    node_idx newresult;

    //`Tricky' zero case

    if (p == 0) {
	result = NewNode(k);
	for (int i = 0; i <= maxVals[k]; i++)
	    SetArc(k, result, i, InternalComplement(k - 1, 0));
	newresult = CheckIn(k, result);
	ComplementCache[k]->add(newresult, p);
	ComplementCache[k]->add(p, newresult);
	return newresult;
    }

    result = NewNode(k);

    Node *nodeP = &FDDL_NODE(k, p);

    int psize = nodeP->size;

    //If the node is not sparse, do things the easy way.
    if (!IS_SPARSE(nodeP)) {
	for (arc_idx i = 0; i <= maxVals[k]; i++) {
	    node_idx u = i < psize ? InternalComplement(k - 1,
							FULL_ARC(k,
								 nodeP,
								 i))
		: InternalComplement(k - 1, 0);
	    SetArc(k, result, i, u);
	}

	newresult = CheckIn(k, result);

//              if (k > 0 && newresult)
//                      FDDL_NODE(k, newresult).flags |= CHECKED_IN;

	ComplementCache[k]->add(newresult, p, 1);
	ComplementCache[k]->add(p, newresult, 1);
    } else {
	//If the node is sparse, do things the fast way!
	int i = 0;
	int ival, idx;

	while (i < psize) {
	    ival = SPARSE_INDEX(k, nodeP, i);
	    idx = SPARSE_ARC(k, nodeP, i);
	    SetArc(k, result, ival, InternalComplement(k - 1, idx));
	    i++;
	}
	for (i = 0; i <= maxVals[k]; i++) {
	    if (FULL_ARC(k, &FDDL_NODE(k, result), i) == 0)
		SetArc(k, result, i, InternalComplement(k - 1, 0));
	}
	newresult = CheckIn(k, result);

//              if (k > 0 && newresult)
//                      FDDL_NODE(k, newresult).flags |= CHECKED_IN;

	ComplementCache[k]->add(newresult, p, 1);
	ComplementCache[k]->add(p, newresult, 1);
    }
    return newresult;
}

node_idx Forest::InternalBComplement(level k, node_idx p)
{
    //Easy Terminal Cases
    if (k == 0) {
	if (p == 0)
	    return 1;
	return 0;
    }
    //Check for an entry in the Cache.
    node_idx result;

    result = BComplementCache[k]->hit(p);
    if (result >= 0) {
	if (!(FDDL_NODE(k, result).flags & DELETED))
	    return result;
	return CheckIn(k, result);
    }

    node_idx newresult;

    //`Tricky' zero case

    if (p == 0) {
	result = NewNode(k);
	for (int i = 0; i <= maxVals[k]; i++)
	    SetArc(k, result, i, InternalBComplement(k - 1, 0));
	newresult = CheckIn(k, result);
	BComplementCache[k]->add(newresult, p);
	BComplementCache[k]->add(p, newresult);
	return newresult;
    }

    result = NewNode(k);

    Node *nodeP = &FDDL_NODE(k, p);

    int psize = nodeP->size;

    //If the node is not sparse, do things the easy way.
    if (!IS_SPARSE(nodeP)) {
	for (arc_idx i = 0; i <= maxVals[k]; i++) {
	    node_idx u = i < psize ? InternalBComplement(k - 1,
							 FULL_ARC(k,
								  nodeP,
								  i))
		: InternalBComplement(k - 1, 0);
	    SetArc(k, result, i, u);
	}

	newresult = CheckIn(k, result);

//              if (k > 0 && newresult)
//                      FDDL_NODE(k, newresult).flags |= CHECKED_IN;

    } else {
	//If the node is sparse, do things the fast way!
	int i = 0;
	int ival, idx;

	while (i < psize) {
	    ival = SPARSE_INDEX(k, nodeP, i);
	    idx = SPARSE_ARC(k, nodeP, i);
	    SetArc(k, result, ival, InternalBComplement(k - 1, idx));
	    i++;
	}
	for (i = 0; i <= maxVals[k]; i++) {
	    if (FULL_ARC(k, &FDDL_NODE(k, result), i) == 0)
		SetArc(k, result, i, InternalBComplement(k - 1, 0));
	}
	newresult = CheckIn(k, result);

//              if (k > 0 && newresult)
//                      FDDL_NODE(k, newresult).flags |= CHECKED_IN;
    }

    BComplementCache[k]->add(newresult, p);
    BComplementCache[k]->add(p, newresult);
    return newresult;
}

int Forest::DestroyMDD(MDDHandle mdd)
{
#ifdef BRIEF_DEBUG
    printf("Destroy MDD: %d\n", mdd.index);
#endif
    if (mdd.index <= 0)
	return INVALID_MDD;
    FDDL_NODE(K, mdd.index).in--;
    if (FDDL_NODE(K, mdd.index).in < 1) {
	DeleteDownstream(K, mdd.index);
    }
    CompactCounter++;
    if (CompactCounter > 100) {
	for (level k = K; k > 0; k--) {
	    Compact(k);
	    CompactCounter = 0;
	}
    }
    return SUCCESS;
}

void Forest::ReallocHandle(MDDHandle & ref)
{
    if (ref.index > 0) {
	DestroyMDD(ref);
    }
}

int Forest::Value(MDDHandle hand, int *tup, int &result)
{
    if (hand.index < 0)
	return INVALID_MDD;
    result = Value(K, hand.index, tup);
    return SUCCESS;
}

int Forest::Value(level k, node_idx p, int *tup)
{
    if (k == 0)
	return p;
    if (p == 0)
	return 0;
    Node *nodeP = &FDDL_NODE(k, p);

    if (IS_SPARSE(nodeP)) {
	for (arc_idx i = 0; i < nodeP->size; i++)
	    if (SPARSE_INDEX(k, nodeP, i) == tup[k])
		return Value(k - 1, SPARSE_ARC(k, nodeP, i), tup);
	return 0;
    } else {
	if (tup[k] < nodeP->size)
	    return Value(k - 1, FULL_ARC(k, nodeP, tup[k]), tup);
	return 0;
    }
}

int Forest::GetMaxVal(level k)
{
    if (k >= 0 && k <= K)
	return maxVals[k];
    else
	return INVALID_LEVEL;
}

int Forest::ChangeMaxVal(level k, int maxval)
{
    assert(maxval >= 0);	//To "disable" range checking, the user
    assert(0 <= k && k <= K);	//should specify a range of "INT_MAX".
    if (maxval == INT_MAX) {
	maxVals[k] = maxval;
	return SUCCESS;
    }

    if (k > 0) {
	for (node_idx i = 0; i < last[k]; i++) {
	    Node *nodeP = &FDDL_NODE(k, i);

	    if (IS_SPARSE(nodeP)) {
		for (int j = 0; j < nodeP->size; j++)
		    if (SPARSE_ARC(k, nodeP, j) > maxval)
			return TUPLE_OUT_OF_BOUNDS;
	    } else if (nodeP->size > maxval)
		return TUPLE_OUT_OF_BOUNDS;
	}
    } else {
	for (int j = 0; j < tail[1]; j++) {
	    if ((*(*arcs[1])[j]) > maxval)
		return TUPLE_OUT_OF_BOUNDS;
	}
    }
    maxVals[k] = maxval;
    return SUCCESS;
}

int Forest::FindRange(level k)
{
    int i;
    int maxVal;
    Node *nodeP;
    maxVal = 0;
    for (i = 1; i < last[k]; i++) {
	nodeP = &FDDL_NODE(k, i);
	if (IS_SPARSE(nodeP)) {
	    if (SPARSE_ARC(k, nodeP, nodeP->size - 1) > maxVal)
		maxVal = SPARSE_INDEX(k, nodeP, nodeP->size - 1);
	} else {
	    if (nodeP->size > maxVal)
		maxVal = nodeP->size;
	}
    }
    return maxVal;
}

//Bring level "kold" to the top of the MDD.
int Forest::Shift(MDDHandle h, level kold, MDDHandle & result)
{
    int temp;
    if (h.index < 0)
	return COMPLEMENT_FAILED;

    node_idx newresult;

    for (level k = K; k > 0; k--)
	ShiftCache[k]->clear();

    level current = kold;
    newresult = h.index;
    while (current < K) {
	//Swap level "current" with level "current+1".
	newresult = InternalShift(K, newresult, current + 1);
	current++;
    }

    if (result.index != newresult) {
	ReallocHandle(result);
	Attach(result, newresult);
    }
    return SUCCESS;
}

node_idx Forest::InternalShift(level k, node_idx p, level target)
{
    node_idx r;
    int maxVal;
    Node *nodeP;

    if (p == 0)
	return 0;
    if (k == 0)
	return p;		//Probably Not Correct.

    nodeP = &FDDL_NODE(k, p);

    r = ShiftCache[k]->hit(p, target);
    if (r >= 0)
	return r;

    r = NewNode(k);

    if (k > target) {
	for (int i = 0; i < nodeP->size; i++) {
	    node_idx m;
	    m = FDDL_ARC(k, nodeP, i);
	    SetArc(k, r, i, InternalShift(k - 1, m, target));
	}
	r = CheckIn(k, r);
	ShiftCache[k]->add(r, p, target);
	return r;
    }

    maxVal = FindRange(k - 1);

#ifndef NON_DEBUG
    printf("Range(%d): %d\n", k - 1, maxVal);
#endif

    for (int val = 0; val < maxVal; val++) {
	node_idx t;
	t = NewNode(k - 1);
#ifndef NON_DEBUG
	printf("Created node: %d\n", t);
#endif
	for (int i = 0; i < nodeP->size; i++) {
	    node_idx j;
	    node_idx n;
	    j = FDDL_ARC(k, nodeP, i);
#ifndef NON_DEBUG
	    printf("<%d,%d>[%d] = %d\n", k, p, i, j);
#endif
	    Node *nodeJ = &FDDL_NODE(k - 1, j);
	    if (val < nodeJ->size) {
		n = FDDL_ARC(k - 1, nodeJ, val);
#ifndef NON_DEBUG
		printf("<%d,%d>[%d] = %d\n", k - 1, j, val, n);
#endif
		SetArc(k - 1, t, i, n);
#ifndef NON_DEBUG
		printf("Setting Arc from <%d,%d>[%d] to %d\n",
		       k - 1, t, i, n);
#endif
	    }
	}
	t = CheckIn(k - 1, t);
#ifndef NON_DEBUG
	printf("Checked in Node.  Result: %d\n", t);
#endif
	SetArc(k, r, val, t);
#ifndef NON_DEBUG
	printf("Setting Arc from <%d,%d>[%d] to %d\n", k, r, val, t);
#endif
    }
    r = CheckIn(k, r);
#ifndef NON_DEBUG
    printf("Checked in Node.  Result: %d\n", r);
#endif
    ShiftCache[k]->add(r, p, target);
    return r;
}
