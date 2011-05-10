/*
 * FDDL : The Free Decision Diagram Library 
 * Copyright (C) 2004 Robert Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

#ifndef FDDL_MDDTYPES_H
#   define FDDL_MDDTYPES_H  1

#   include <iostream>
#   include <assert.h>
using namespace std;

typedef int potential_range_element;	//Local state index, not yet confirmed to be globally reachable.
typedef int range_element;		  //Globally reachable local state index.
typedef int event;				  //Event index
typedef int level;				  //Level index
typedef int node_idx;			  //MDD Node index
typedef int arc_idx;				  //MDD Arc index

class   mdd_handle {
	friend class fddl_forest;
 protected:

 public:

   int     index;

   mdd_handle() {
      index = -1;
   } 
   bool  
   isEqual(mdd_handle & b) 
   {
      return b.index == index;
   }

   bool operator==(mdd_handle a) 
   {
      cout << "Error in handle for MDD " << a.index << ":\n";
      cout << "Error: Cannot compare MDD handles with `=='.  Use isEqual\n";
      return false;
   }
   
   mdd_handle operator=(mdd_handle a) 
   {
      cout << "Error:  Cannot assign MDD handles using =" << endl;
      assert(0);
      return a;
   }

   void RemapHandle(node_idx newidx) {
      index = newidx;
   }
};
#endif
