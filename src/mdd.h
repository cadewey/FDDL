/*
 * FDDL : The Free Decision Diagram Library 
 * Copyright (C) 2004 Robert Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

//#define FDDL_DEBUG
//#define BRIEF_DEBUG

#ifndef __MDD_H
#define __MDD_H

#include <iostream>
#include <assert.h>

#include "mddhandle.h"
#include "caches.h"
#include "dynarray.h"

using namespace std;

enum garbage_algorithm {O_LAZY=0, O_STRICT=2}; 
enum flags {SHARED=1,SPARSE=2,DELETED=4,CHECKED_IN=8,SATURATED=16};
enum mdd_op_return_val { SUCCESS = 0, TUPLE_OUT_OF_BOUNDS, INVALID_MDD, MAX_FAILED, MIN_FAILED, COMPLEMENT_FAILED, INVALID_LEVEL };

class UniqueTable;

struct label {
    int rule_num;
    label *next;
};

//An MDD node consists of two parts, a node struct stored in a node
//array, and a set of arcs stored in a per-level arc array.
struct Node {
    char flags;			//Parameter flags (see above for values)
    int down;			//Index into the arc array of the first downpointer for this node
    int size;			//Number of arcs leaving this node.  If stored sparsely, 
    //the number of non zeros.
    int in;			//Number of incoming Arcs
     Node() {
	flags = 0;
	down = -1;
	size = 0;
	in = 0;
}};


class print_node {
    int K;

  public:
    int *low;
    int *high;
    print_node *next;
    int *mV;

     print_node(int num_levels, int *maxVals) {
	K = num_levels;
	low = new int[K + 1];
	 high = new int[K + 1];
	 mV = maxVals;
    } ~print_node() {
	delete[]low;
	delete[]high;
    }

    void Print(int *mask) {
	char ch;
	for (int k = K; k > 0; k--) {
	    if (mask[k] == 1) {
		if (k == 14) {
		    if (low[k] == 0 && high[k] == mV[k]) {
			cout << ("ALL ");
			continue;
		    } else {
			for (int i = low[k]; i <= high[k]; i++) {
			    if (i == 0) {
				cout << ("ICMP ");
			    } else if (i == 1) {
				cout << ("UDP ");
			    } else if (i == 2) {
				cout << ("TCP ");
			    }
			}
			continue;
		    }
		}
		if (k == 13 || k == 11) {
		    if (low[k] == 0 && high[k] == mV[k] && low[k - 1] == 0
			&& high[k - 1] == mV[k - 1]) {
			cout << ("*");
		    } else if (low[k] == high[k]
			       && low[k - 1] == high[k - 1])
			cout << low[k] * 256 + low[k - 1];
		    else
			cout << low[k] * 256 + low[k -
						   1] << "-" << high[k] *
			    256 + high[k - 1];
		    continue;
		}
		if (k == 12 || k == 10)
		    continue;

		ch = ' ';
		if (k <= 22 && k >= 20)
		    ch = '.';
		if (k <= 18 && k >= 13)
		    ch = '.';
		if (low[k] == high[k]) {
		    cout << "[" << high[k] << "]" << ch << endl;
		} else if (low[k] == 0 && high[k] == mV[k]) {
		    cout << ch;
		} else {
		    cout << "[" << low[k] << "-" << high[k] << "]" << ch;
		}
	    }
	}
	cout << " ";
    }
};


//The class fddl_forest stores a forest of MDDs with K levels, where K is
//chosen by the user.  By using a node-sharing technique, we store
//multiple MDDs relatively efficiently. 


class fddl_forest {
  protected:
    int K;			//Number of Non-terminal Levels
    int *tail;			//Array [1..K] which records number of arcs per level.
    node_idx *last;		//Array [1..K] which records number of nodes per level.
    int *maxVals;		//Array [0..K] of maximum values for each level.

    int CompactCounter;

    //Caches for embedded operations 
    Cache **ProjectCache;	
    Cache **PruneCache;		
    Cache **RestrictCache;
    Cache **MaxCache;
    Cache **MinCache;
    Cache **ComplementCache;
    Cache **BComplementCache;
    Cache **LessThanCache;	
    Cache **ValRestrictCache;	
    Cache **CombineCache;
    Cache **ReplaceCache;
    Cache **ProjectOntoCache;
    Cache **ReplaceStrictCache;
    Cache **ShiftCache;
    Cache **PrintCache;
    TupleCache **ApplyCache;
    TupleCache **SelectCache;

    int Value(level k, node_idx p, int *tup);

  public:

    void ReallocHandle(MDDHandle& ref);

    void Attach(MDDHandle& ref, node_idx i) {
	ref.index = i;
	(*(*nodes[K])[i]).in++;
    } DynArray < node_idx > **node_remap_array;

    DynArray < Node > **nodes;	//An array [1..K] of heaps of MDD nodes.
    DynArray < node_idx > **arcs;	//An array [1..K] of heaps of MDD arcs.
    DynArray < label * >**labels;	//An array [1..K] of labels for each arc.

    //Build a new MDD forest of numlevels levels.
    //The domain of each level is specified as an integer range
    //from 0 to maxvals[k] inclusive.

    unsigned int hashnode(const level k, const node_idx p) const;
    int compare(const level k, const node_idx p, const node_idx q) const;

    fddl_forest(int numlevels, int *maxvals);
    ~fddl_forest();

    int Last(level k) {
	return last[k];
    }

    int GetMaxVal(level k);
    int ChangeMaxVal(level k, int maxval);

    int MakeMDDFromTuple(int *low, int *high, MDDHandle& ref);
    int Assign(MDDHandle root, int *low, int *high, MDDHandle & result);
    int DestroyMDD(MDDHandle mdd);
    int IsElementOf(MDDHandle root, int *tup, bool & result);
    int Value(MDDHandle root, int *tup, int &result);

    int Max(MDDHandle p, MDDHandle q, MDDHandle & result);
    int Min(MDDHandle p, MDDHandle q, MDDHandle & result);
    int Complement(MDDHandle p, MDDHandle & result);
    int BinaryComplement(MDDHandle p, MDDHandle & result);
    int LessThan(MDDHandle p, int value, MDDHandle & result);
    int Apply(MDDHandle * roots, int num_roots,
	      node_idx(*func) (node_idx *, int), MDDHandle & result);
    int ValRestrict(MDDHandle p, int value, MDDHandle & result);
    int Combine(MDDHandle p, MDDHandle q, int cache_index,
		MDDHandle & result);
    int Select(MDDHandle p, int num_chains, MDDHandle * chains,
	       MDDHandle & result);

    int Shift(MDDHandle p, level kold, MDDHandle & result);

    int Replace(MDDHandle p, MDDHandle q, bool strict,
		MDDHandle & result);

    int ProjectOnto(MDDHandle p, MDDHandle q, MDDHandle & result);

    void PrintVals(MDDHandle root, level k);
    node_idx ProjectVals(level k, node_idx p, level cutoff);
    node_idx Projection(level k, node_idx p, level * mask);
    int InternalPrintVals(level k, node_idx p);
    void PrintPort(MDDHandle root, level k);
    void PrintRanges(MDDHandle root, level * mask);
    void PrintRanges(level k, node_idx p, level * mask,
		     print_node * &stack, int *low, int *high);
    void PrintAddy(MDDHandle root, level k);
    void PrintAddy(level k, node_idx p, int *vals, int depth);
    void PrintStates(node_idx root);
    void PrintStates(level k, node_idx root, int *states);

    int CountNodes(node_idx);
    int CountNodes(level k, node_idx p);
    int ComputeMemUsed();
    int RealMemUsed(node_idx);
    int RealMemUsed(level k, node_idx p);

    void ToggleSparsity(bool SparseSwitch);
    void SetGarbageCollection(int alg, int threshold);
    void PruneMDD(MDDHandle p);
    void PruneMDD(node_idx p);
    void PrintMDD();
    void PrintMDD(int top, int bottom);

    //Functions to create, manipulate, and destroy MDD nodes.
    node_idx NewNode(level k);
    void DeleteNode(level k, node_idx p);
    node_idx CheckIn(level k, node_idx p);
    int UnpackNode(level k, arc_idx p, int *&fullarray);
    void DeleteDownstream(level k, node_idx p);

    //Switch levels k1 and k2 in the MDD (maybe save space or time?)
    typedef struct state_triple {
	arc_idx from;
	arc_idx to;
	node_idx child;
    } state_triple;

    //Set <k:p>[i] = j, updating incoming arc information in the node
    //which the arc currently points to as well as the node we are 
    //assigning the arc to.
    void SetArc(level k, node_idx p, arc_idx i, node_idx j);

    //Do Garbage Collection
    void Compact(level k);
    void CompactTopLevel();

    UniqueTable *UT;		//A special hashtable, the unique table, for storing

    //node information.

    node_idx InternalReplace(level k, node_idx p, node_idx q);
    node_idx InternalProjectOnto(level k, node_idx p, node_idx q);
    node_idx InternalReplaceStrict(level k, node_idx p, node_idx q);
    node_idx InternalRestrict(level k, node_idx p, node_idx q);
    node_idx InternalMax(level k, node_idx p, node_idx q);
    node_idx InternalMin(level k, node_idx p, node_idx q);
    node_idx InternalComplement(level k, node_idx p);
    node_idx InternalBComplement(level k, node_idx p);
    node_idx InternalLessThan(level k, node_idx p, int value);
    node_idx InternalApply(level k, node_idx * roots, int numroots,
			   node_idx(*func) (node_idx *, int));
    node_idx InternalValRestrict(level k, node_idx p, int value);
    node_idx InternalCombine(level k, node_idx p, node_idx q,
			     int chain_index);
    node_idx InternalSelect(level k, node_idx p, int num_chains,
			    node_idx * all_chains);
    node_idx InternalShift(level k, node_idx p, level target);

    int FindRange(level k);
    void InternalPruneMDD(level k, node_idx p, int flag);
    void FlushCaches(level k);

    int garbage_alg;
    int garbage_threshold;
    bool sparseEnabled;

    void SaveMDD(char *filename);
    void LoadMDD(char *filename);

    inline Node& FDDL_NODE(level k, node_idx p) const
    {
	    return *((*nodes[k])[p]);
    }

    inline node_idx& FULL_ARC(level k, Node* n, arc_idx i) const
    {
	return *((*arcs[k])[n->down + i]);
    }

    inline label* FULL_LABEL(level k, Node* n, arc_idx i) const
    {
	return *((*labels[k])[n->down + i]);
    } 

    inline arc_idx& SPARSE_INDEX(level k, Node* n, arc_idx i) const
    {
	return *((*arcs[k])[n->down + 2 * i]);
    }
    
    inline node_idx& SPARSE_ARC(level k, Node* n, arc_idx i) const
    {
	return *((*arcs[k])[n->down + 2 * i + 1]);
    }

    inline label* LABEL_INDEX(level k, Node* n, arc_idx i) const
    {
	return *((*labels[k])[n->down + 2 * i]);
    }

    inline label* SPARSE_LABEL(level k, Node* n, arc_idx i) const
    {
	return *((*labels[k])[n->down + 2 * i + 1]);
    }

   inline bool IS_SPARSE(const Node* const n) const
   {
	return (n->flags & SPARSE) > 0;
   }

   inline bool IS_DELETED(const Node* const n) const
   {
	return (n->flags & DELETED) > 0;
   }

   inline node_idx& FDDL_ARC(level k, Node* n, arc_idx i) const
   {
	return IS_SPARSE(n) ? SPARSE_ARC(k,n,i) : FULL_ARC(k,n,i);
   }
};

#endif //__MDD_H
