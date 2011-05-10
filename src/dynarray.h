/*
 * FDDL : The Free Decision Diagram Library 
 * Copyright (C) 2004 Robert Marmorstein
 * 
 * This program is released under the GNU Public License, version 2.  Please
 * see the file "LICENSE" in the root directory of this repository for more
 * information.
 */

//Default was 2, but why not do 4 and spend less time allocating?

template < typename T > class dynarray {
	T     **data;
	int     size;
	T      *defValue;
        int mag;

private:
    void 
    extend()
    {
        T     **newData;
        newData = new T *[size * mag];
        assert(newData);
        for (int i = 0; i < size; i++) {
            newData[i] = data[i];  //Pointer assignment, not value assignment!
        }
        delete[]data;
        for (int i = size; i < size * mag; i++) {
            newData[i] = new T;
            if (defValue)
                (*newData[i]) = *defValue;
        }
        data = newData;
        size *= mag;
        newData = NULL;
	mag++;
    }

public:
    dynarray(T const &def)
    {
        data = new T *[256];
        size = 256;
        defValue = new T;
        *defValue = def;
        for (int i = 0; i < size; i++) {
            data[i] = new T;
            *(data[i]) = def;
        }
	mag = 2;
    }

    dynarray()
    {
	mag = 2;
        data = new T *[256];
        size = 256;
        for (int i = 0; i < size; i++)
            data[i] = new T;
        defValue = NULL;
    }
    
    ~dynarray()
    {
        for (int i = 0; i < size; i++)
	{
            if (data[i])
                delete  data[i];
            data[i] = NULL;
        }
        delete[]data;
        data = NULL;
        size = -1;
        if (defValue)
            delete  defValue;
        defValue = NULL;
    }

    T *&operator[] (int index)
    {
        while (index >= size)
            extend();
        return data[index];
    }
};
#endif
