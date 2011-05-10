/*
 * FDDL : The Free Decision Diagram Library 
 * Copyright (C) 2004 Robert Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

//Default was 2, but why not do 4 and spend less time allocating?

template < typename T > class Dynarray {
	private:
		T     **m_data;
		int     m_size;
		T      *m_defValue;
	        int     m_mag;

private:
    void 
    extend()
    {
        T     **newData;
        newData = new T *[m_size * m_mag];
        assert(newData);
        for (int i = 0; i < m_size; i++) {
            newData[i] = m_data[i];  //Pointer assignment, not value assignment!
        }
        delete[] m_data;
        for (int i = m_size; i < m_size * m_mag; i++) {
            newData[i] = new T;
            if (m_defValue)
                (*newData[i]) = *m_defValue;
        }
        m_data = newData;
        m_size *= m_mag;
        newData = NULL;
	m_mag++;
    }

public:
    Dynarray(T const &def)
    {
        m_data = new T *[256];
        m_size = 256;
        m_defValue = new T;
        *m_defValue = def;
        for (int i = 0; i < m_size; i++) {
            m_data[i] = new T;
            *(m_data[i]) = def;
        }
	m_mag = 2;
    }

    Dynarray()
    {
	m_mag = 2;
        m_data = new T *[256];
        m_size = 256;
        for (int i = 0; i < m_size; i++)
            m_data[i] = new T;
        m_defValue = NULL;
    }
    
    ~Dynarray()
    {
        for (int i = 0; i < m_size; i++)
	{
            if (m_data[i])
                delete m_data[i];
            m_data[i] = NULL;
        }
        delete[] m_data;
        m_data = NULL;
        m_size = -1;
        if (m_defValue)
            delete  m_defValue;
        m_defValue = NULL;
    }

    T *&operator[] (int index)
    {
        while (index >= m_size)
            extend();
        return m_data[index];
    }
};
#endif
