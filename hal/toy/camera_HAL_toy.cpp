#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <errno.h>
#include <iostream>
#include <cstdio>
#include <unistd.h>

#include <hardware.h>

#include "camera_HAL_toy.h"
#include "ControlThread.h"

static ControlThread *control_thread;

using std::cout;
using std::endl;

int toy_camera_open(void)
{
    cout << "toy_camera_open" << endl;

    control_thread = new ControlThread();

    if (control_thread == NULL) {
        cout << "Memory allocation error!" << endl;
        return -ENOMEM;
    }

    int status = control_thread->init();
    if (status != ERROR_NONE) {
        printf("Error initializing ControlThread");
        return -1;
    }

    control_thread->Start();

    return 0;
}

int toy_camera_dump(void)
{
    control_thread->dump();
    return 0;
}

int toy_camera_take_picture(void)
{
    return control_thread->takePicture();
}

int toy_camera_set_callbacks(camera_notify_callback notify_cb,
        camera_data_callback data_cb)
{
    if (!control_thread)
        return BAD_VALUE;

    control_thread->setCallbacks(notify_cb, data_cb);

    return OK;
}

hw_module_t HAL_MODULE_INFO_SYM = {
    tag: HARDWARE_MODULE_TAG,
    id: CAMERA_HARDWARE_MODULE_ID,
    name: "Toy Camera Hardware Module",
    open: toy_camera_open,
    take_picture: toy_camera_take_picture,
    dump: toy_camera_dump,
    set_callbacks: toy_camera_set_callbacks,
};
