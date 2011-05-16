#include <iostream>
#include "../src/cache.h"

using namespace std;

Cache c;

int test_single(){
	c.add(5,0);
	c.add(4,1);
	c.add(3,2);
	c.add(2,3);
	c.add(1,4);
	c.add(0,5);

	//Check that cached results give the correct values.
	if (c.hit(5) != 0)
		return -1;
	if (c.hit(4) != 1)
		return -1;
	if (c.hit(3) != 2)
		return -1;
	if (c.hit(2) != 3)
		return -1;
	if (c.hit(1) != 4)
		return -1;
	if (c.hit(0) != 5)
		return -1;
	
	//Check that uncached results give -1.
	if (c.hit(6) != -1)
		return -1;
	return 0;
}

int test_double(){
	c.add(0,0,0);
	c.add(1,0,1);
	c.add(1,1,0);
	c.add(0,1,1);

	//Check that cached results give the correct values.
	if (c.hit(0,0) != 0)
		return -1;
	if (c.hit(0,1) != 1)
		return -1;
	if (c.hit(1,0) != 1)
		return -1;
	if (c.hit(1,1) != 0)
		return -1;

	//Check that uncached results give -1.
	if (c.hit(0,2) != -1)
		return -1;
	if (c.hit(2,0) != -1)
		return -1;

	return 0;
}

int test_triple(){
	c.add(0,0,0,0);
	c.add(0,0,0,1);
	c.add(0,0,1,0);
	c.add(0,0,1,1);
	c.add(0,1,0,0);
	c.add(0,1,0,1);
	c.add(0,1,1,0);
	c.add(1,1,1,1);

	//Check that cached results give the correct values.
	if (c.hit(0,0,0) != 0){
		cout << "(0,0,0)" << endl;
		return -1;
	}
	if (c.hit(0,0,1) != 0){
		cout << "(0,0,1)" << endl;
		return -1;
	}
	if (c.hit(0,1,0) != 0){
		cout << "(0,1,0)" << endl;
		return -1;
	}
	if (c.hit(0,1,1) != 0){
		cout << "(0,1,1)" << endl;
		return -1;
	}
	if (c.hit(1,0,0) != 0){
		cout << "(1,0,0) = " << c.hit(1,0,0) << endl;
		return -1;
	}
	if (c.hit(1,0,1) != 0){
		cout << "(1,0,1)" << endl;
		return -1;
	}
	if (c.hit(1,1,0) != 0){
		cout << "(1,1,0)" << endl;
		return -1;
	}
	if (c.hit(1,1,1) != 1){
		cout << "(1,1,1) != 1" << endl;
		return -1;
	}

	//Check that uncached results give -1.
	if (c.hit(2,0,0) != -1){	
		cout << "Uncached result!" << endl;
		return -1;
	}

	return 0;
}

int test_clear(){
	c.add(3,2);
	c.add(1,0,1);
	c.add(0, 1,1,0);
	
	c.clear();

	//Make sure that cached results have been cleared
	if (c.hit(2) != -1)
		return -1;
	if (c.hit(0,1) != -1)
		return -1;

	if (c.hit(1,1,0) != -1)
		return -1;

	return 0;
}

int main(int argc, char** argv)
{
	if (test_single()){
		cout << "Test Single Failed." << endl;
		return -1;
	}
	if (test_double()){
		cout << "Test Double Failed." << endl;
		return -1;
	}
	if (test_triple()){
		cout << "Test Triple Failed." << endl;
		return -1;
	}
	if (test_clear()){
		cout << "Test Clear Failed." << endl;
		return -1;
	}
	return 0;
}
