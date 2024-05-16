#include <iostream>
#include <cstdio>
#include <unistd.h>

#include "ControlThread.h"


using std::cout;
using std::endl;


ControlThread::ControlThread()
{
	cout << "C++ constuctor" << endl;

}

ControlThread::~ControlThread()
{

	cout << "C++ destructor" << endl;


}

int ControlThread::takePicture()
{



	cout << "c++ takePicture()" << endl;
	return 0;
}


int ControlThread::dump()
{

    cout << "C++ connection : camera dump" << endl;

    return 0;

}
