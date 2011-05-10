/*
 * FDDL : The Free Decision Diagram Library 
 * Copyright (C) 2004 Robert Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

#ifndef __DYNARRAY_H
#define __DYNARRAY_H

#include <cstdlib>
#include <assert.h>

using namespace std;

const int DEFAULT_SIZE = 256;
const int DEFAULT_SCALE = 2;

template < typename T > class DynArray {
  private:
    T ** m_data;
    unsigned int m_size;
    unsigned int m_scale;
    T *m_default_value;

  public:
    DynArray() : m_size(DEFAULT_SIZE), m_scale(DEFAULT_SCALE), m_default_value(NULL) 
    {
	m_data = new T *[DEFAULT_SIZE];
	for (unsigned int i = 0; i < m_size; i++){
	    m_data[i] = new T;
	}
    }

    DynArray(T const &def) : m_size(DEFAULT_SIZE), m_scale(DEFAULT_SCALE) 
    {
	m_data = new T *[DEFAULT_SIZE];
	m_default_value = new T;

	for (unsigned int i = 0; i < m_size; i++) {
	    m_data[i] = new T;
	    *(m_data[i]) = def;
	}
    }

    ~DynArray() {
	for (unsigned int i = 0; i < m_size; i++) {
	    if (m_data[i])
		delete m_data[i];
	}
	delete[] m_data;
	if (m_default_value)
	    delete m_default_value;
	m_default_value = NULL;
    }

    T *&operator[] (const unsigned int index) {
	while (index >= m_size){
	    extend();
	}
	return m_data[index];
    }
  
  private:
    void extend() {
	T **newData = new T *[m_size * m_scale];
	assert(newData);
	for (unsigned int i = 0; i < m_size; i++) {
	    newData[i] = m_data[i];	//Pointer assignment, not value assignment!
	}
	delete[] m_data;
	for (int i = m_size; i < m_size * m_scale; i++) {
	    newData[i] = new T;
	    if (m_default_value)
		(*newData[i]) = *m_default_value;
	}
	m_data = newData;
	m_size *= m_scale;

	//Since we needed to extend, perhaps the scale is too small.  Make it bigger.
	m_scale++;
    }
};
#endif				//__DYNARRAY_H
