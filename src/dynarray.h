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

const int DEFAULT_SCALE = 2;
const int DEFAULT_SIZE = 256;

template < typename T > class Dynarray {
  private:
    T ** m_data;
    int m_size;
    T *m_defValue;
    int m_scale;

  public:
    Dynarray(T const &def) {
	m_data = new T *[DEFAULT_SIZE];
	m_size = DEFAULT_SIZE;
	m_defValue = new T;
	*m_defValue = def;
	for (int i = 0; i < m_size; i++) {
	    m_data[i] = new T;
	    *(m_data[i]) = def;
	}
	m_scale = DEFAULT_SCALE;
    }

    Dynarray() {
	m_scale = DEFAULT_SCALE;
	m_data = new T *[DEFAULT_SIZE];
	m_size = DEFAULT_SIZE;
	for (int i = 0; i < m_size; i++)
	    m_data[i] = new T;
	m_defValue = NULL;
    }

    ~Dynarray() {
	for (int i = 0; i < m_size; i++) {
	    if (m_data[i])
		delete m_data[i];
	    m_data[i] = NULL;
	}
	delete[]m_data;
	m_data = NULL;
	m_size = -1;
	if (m_defValue)
	    delete m_defValue;
	m_defValue = NULL;
    }

    T *&operator[] (int index) {
	while (index >= m_size)
	    extend();
	return m_data[index];
    }

  private:
    void extend() {
	T **newData;
	newData = new T *[m_size * m_scale];
	assert(newData);
	for (int i = 0; i < m_size; i++) {
	    newData[i] = m_data[i];	//Pointer assignment, not value assignment!
	}
	delete[]m_data;
	for (int i = m_size; i < m_size * m_scale; i++) {
	    newData[i] = new T;
	    if (m_defValue)
		(*newData[i]) = *m_defValue;
	}
	m_data = newData;
	m_size *= m_scale;
	newData = NULL;
	m_scale++;
    }

};
#endif				//__DYNARRAY_H
