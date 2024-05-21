#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <errno.h>
#include <iostream>
#include <cstdio>
#include <unistd.h>


#include <hardware.h>

#include "camera_HAL_oem.h"
#include "ControlThread.h"


static ControlThread *control_thread;

using namespace std;


int oem_camera_open(void)
{

    cout << "toy_camera_open" << endl;

    control_thread = new ControlThread();

    if(control_thread == NULL) {
	cout << "Memory allocation error! " <<endl;
	return -ENOMEM;


    }

    return 0;


}


int oem_camera_take_picture(void)
{


    return control_thread->takePicture();


}


int oem_camera_dump(void)
{

    return control_thread->dump();

}


hw_module_t HAL_MODULE_INFO_SYM = {

    tag: HARDWARE_MODULE_TAG,
    id: CAMERA_HARDWARE_MODULE_ID,
    name: "OEM Camera Hardware Module",
    open: oem_camera_open,
    take_picture: oem_camera_take_picture,
    dump: oem_camera_dump,

};
