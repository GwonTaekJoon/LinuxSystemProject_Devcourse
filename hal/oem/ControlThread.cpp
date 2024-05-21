#include <iostream>

#include "ControlThread.h"


using namespace std;

ControlThread::ControlThread()
{

    cout << "OEM: C++ AREA" << endl;

}

ControlThread::~ControlThread()
{

    cout << "OEM: destructor" << endl;

}


int ControlThread::takePicture()
{

    cout << "OEM: Taking picture" << endl;
    return 0;
}


int ControlThread::dump()
{

    cout << "OEM: Camera Dump" << endl;

    return 0;


}
