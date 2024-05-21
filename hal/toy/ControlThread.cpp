#include <iostream>
#include <unistd.h>


#include "ControlThread.h"

using namespace std;


ControlThread::ControlThread()
{

    cout << "TOY: C++ AREA" << endl;


}

ControlThread::~ControlThread()
{

    cout << "TOY: destructor" <<endl;

}

int ControlThread::takePicture()
{

    cout << "TOY: taking picture" << endl;
    return 0;

}

int ControlThread::dump()
{

    cout << "TOY: camera dump" << endl;


    return 0;
}
