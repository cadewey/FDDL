/*
 * FDDL : The Free Decision Diagram Library 
 * Copyright (C) 2004 Robert Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

#ifndef __MDDTYPES_H
#define __MDDTYPES_H

#include <iostream>
#include <assert.h>

using namespace std;

typedef int level;		//Level of the MDD
typedef int node_idx;		//Index of an MDD node in the dynamic node array for a particular level
typedef int arc_idx;		//Index of an MDD arc in the dynamic arc array for a particular level

class mdd_handle {
    friend class fddl_forest;
  protected:

  public:

    int index;

     mdd_handle() {
	index = -1;
    } bool isEqual(mdd_handle & b) {
	return b.index == index;
    }

    bool operator==(mdd_handle a) {
	cout << "Error in handle for MDD " << a.index << ":\n";
	cout <<
	    "Error: Cannot compare MDD handles with `=='.  Use isEqual\n";
	return false;
    }

    mdd_handle operator=(mdd_handle a) {
	cout << "Error:  Cannot assign MDD handles using =" << endl;
	assert(0);
	return a;
    }

    void RemapHandle(node_idx newidx) {
	index = newidx;
    }
};
#endif				//__MDDTYPES_H
