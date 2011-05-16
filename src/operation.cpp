#include "operation.h"
#include "cache.h"
#include "mdd.h"

Operation::Operation (Forest * forest):m_forest (forest)
{
	const unsigned int K = m_forest->top_level();
	m_cache = new Cache *[K + 1];
	for (level k = K; k >0; k--)
		m_cache[k] = new Cache;
}

Operation::~Operation ()
{
	const unsigned int K = m_forest->top_level();
	for (level k = K; k > 0; k--)
		delete m_cache[k];
	delete[] m_cache;
}
