#include "operation.h"
#include "caches.h"
#include "mdd.h"

Operation::Operation (Forest * forest):m_forest (forest)
{
  const unsigned int
    K = m_forest->top_level ();
  m_cache = new Cache *[K + 1];
  for (unsigned int i = 1; i <= K; i++)
    {
      m_cache[i] = new Cache;
    }
}

Operation::~Operation ()
{
  const unsigned int K = m_forest->top_level ();
  for (unsigned int i = 1; i <= K; i++)
    {
      delete m_cache[i];
    }
  delete[]m_cache;
}

node_idx
Operation::operation (level k, node_idx p, node_idx q)
{
  node_idx result;
  result = m_cache[k]->hit (p, q);
  if (result >= 0)
    {
      if (!m_forest->FDDL_NODE (k, result).flags & DELETED)
	{
	  return result;
	}
      if (m_forest->garbage_alg != O_STRICT)
	{
	  return m_forest->CheckIn (k, result);
	}
    }

  result = m_forest->NewNode (k);
  Node *nodeP = &m_forest->FDDL_NODE (k, p);
  Node *nodeQ = &m_forest->FDDL_NODE (k, q);

  int psize = nodeP->size;
  int qsize = nodeQ->size;

  //If neither node is sparse, do things the easy way.
  if (!m_forest->IS_SPARSE (nodeP) && !m_forest->IS_SPARSE(nodeQ))
  {
	for (arc_idx i = 0; i < (psize > qsize ? psize : qsize); i++)
	{
	  node_idx u = operation(k - 1, i < psize ? m_forest->FULL_ARC (k, nodeP, i) : 0, i < qsize ? m_forest->FULL_ARC (k, nodeQ, i) : 0);
	  m_forest->SetArc (k, result, i, u);
	}
  }
  else if (m_forest->IS_SPARSE(nodeP) && m_forest->IS_SPARSE(nodeQ))
  {
      //If both nodes are sparse, do things the fast way!
      //Scan from left to right.  If i is the smaller value, put it in the
      //node.  If j is the smaller value, put it in the node.  If i==j, put
      //the union of i and j in the node.  

	for (arc_idx i = 0, j = 0; i < psize || j < qsize;)
	{
		arc_idx pdx = m_forest->SPARSE_INDEX (k, nodeP, i);
		node_idx pval = m_forest->SPARSE_ARC (k, nodeP, i);
		arc_idx qdx = m_forest->SPARSE_INDEX (k, nodeQ, j);
		node_idx qval = m_forest->SPARSE_ARC (k, nodeQ, j);

		if (i >= psize)
		{
			m_forest->SetArc(k, result, qdx, qval);
			j++;
		}
		else if (j >= qsize)
		{
			m_forest->SetArc(k, result, pdx, pval);
			i++;
		}
		else if (pdx < qdx)
		{
			m_forest->SetArc(k, result, pdx, pval);
			i++;
		}
		else if (qdx < pdx)
		{
			m_forest->SetArc(k, result, qdx, qval);
			j++;
		}
		else
		{
			m_forest->SetArc(k, result, pdx, operation(k - 1, pval, qval));
			i++;
			j++;
		}
	}
  }
  else{
	if (m_forest->IS_SPARSE(nodeP) && !m_forest->IS_SPARSE(nodeQ))
	{
		int j = 0;

		for (int i = 0; i < nodeP->size || j < nodeQ->size;)
		{
			int idx = m_forest->SPARSE_INDEX (k, nodeP, i);
			int ival = m_forest->SPARSE_ARC (k, nodeP, i);
			int jval = m_forest->FULL_ARC (k, nodeQ, j);

		if (i >= nodeP->size)
		{
		  m_forest->SetArc(k, result, j, jval);
		  j++;
		}
		else if (j >= nodeQ->size)
		{
		  m_forest->SetArc(k, result, idx, ival);
		  i++;
		}
		else if (j < idx)
		{
		  m_forest->SetArc(k, result, j, jval);
		  j++;
		}
		else if (idx < j)
		{
		  m_forest->SetArc(k, result, idx, ival);
		  i++;
		}
		else
		{
		  m_forest->SetArc(k, result, j, operation(k - 1, ival, jval));
		  i++;
		  j++;
		}
	    }
	  }
	  else if (m_forest->IS_SPARSE (nodeQ) && !m_forest->IS_SPARSE (nodeP))
	  {
	    int j = 0;

	    for (int i = 0; i < nodeP->size || j < nodeQ->size;)
	      {
		int jdx = m_forest->SPARSE_INDEX (k, nodeQ, j);
		int jval = m_forest->SPARSE_ARC (k, nodeQ, j);
		int ival = m_forest->FULL_ARC (k, nodeP, i);

		if (i >= nodeP->size)
		  {
		    m_forest->SetArc (k, result, jdx, jval);
		    j++;
		  }
		else if (j >= nodeQ->size)
		  {
		    m_forest->SetArc (k, result, i, ival);
		    i++;
		  }
		else if (i < jdx)
		  {
		    m_forest->SetArc (k, result, i, ival);
		    i++;
		  }
		else if (jdx < i)
		  {
		    m_forest->SetArc (k, result, jdx, jval);
		    j++;
		  }
		else
		  {
		    m_forest->SetArc (k, result, i, operation(k - 1, ival, jval));
		    i++;
		    j++;
		  }
	      }

	  }
	}

      node_idx newresult = m_forest->CheckIn (k, result);

      m_cache[k]->add (newresult, p, q);
      m_cache[k]->add (newresult, q, p);
      m_cache[k]->add (newresult, p, newresult);
      m_cache[k]->add (newresult, q, newresult);
      return newresult;
}
