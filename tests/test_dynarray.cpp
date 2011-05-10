#include <iostream>
#include "../src/dynarray.h"

using namespace std;

DynArray<int> array;

bool test_access(){
	*(array[0])=50;
	if (*(array[0]) != 50)
		return false;
	return true;
}

bool test_extend(){
	*(array[1024]) = 9999;
	if (*(array[1024]) != 9999)
		return false;
	return true;
}

bool fac_test(){
	unsigned int fac = 1;
	for (unsigned int i=1;i<11;i++){
		fac = fac * i;
		*(array[fac]) = i;
	}
	fac = 1;
	for (unsigned int i=1; i<11;i++){
		fac = fac * i;
		if (*(array[fac]) != i){
			return false;
		}
	}
	return true;
}

int main(int argc, char** argv)
{
	if (!test_access())
		return -1;

	if (!test_extend())
		return -1;

        if (!fac_test())
		return -1;

	return 0;
}
