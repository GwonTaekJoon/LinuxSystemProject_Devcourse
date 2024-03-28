#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <errno.h>
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include "camera_HAL.h"
#include "ControlThread.h"


static ControlThread *control_thread;

using std::cout;
using std::endl;

int toy_camera_open(void)
{


	cout << "toy_camera_open" << endl;
	control_thread = new ControlThread();

	if(control_thread == NULL) {

		cout << "Memory allocation error!" << endl;
		return -ENOMEM;

	}

	return 0;

 }

int toy_camera_take_picture(void)
{

	return control_thread -> takePicture();

}
