#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "Callbacks"

#include <hardware.h>
#include "Callbacks.h"

Callbacks* Callbacks::mInstance = NULL;

using std::exception;
using std::cout;
using std::cerr;

#define STR_CONCAT(A,B) A << B << ": "
#define __THREAD_FUNC__ STR_CONCAT("[TOY_CB]",__FUNCTION__)

Callbacks::Callbacks():
    mNotifyCB(NULL)
    ,mDataCB(NULL)
{
    cout << __THREAD_FUNC__ << endl;
}

Callbacks::~Callbacks()
{
    cout << __THREAD_FUNC__ << endl;
    mInstance = NULL;
}

void Callbacks::setCallbacks(camera_notify_callback notify_cb,
                                camera_data_callback data_cb)
{
    cout << __THREAD_FUNC__ << ": Notify = " << notify_cb << "Data = " << data_cb << endl;

    mNotifyCB = notify_cb;
    mDataCB = data_cb;
}

void Callbacks::takePictureDone(void)
{
    cout << __THREAD_FUNC__  << endl;

    if (mNotifyCB)
        mNotifyCB(CAMERA_TAKE_PICTURE_DONE, 1, 0);
}
