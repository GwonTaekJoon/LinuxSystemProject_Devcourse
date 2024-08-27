#ifndef _CALLBACKS_
#define _CALLBACKS_

#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <Thread.h>
#include <Timers.h>

#include <hardware.h>

using std::string;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;

class Callbacks {
    static Callbacks* mInstance;
    Callbacks();
public:
    static Callbacks* getInstance() {
        if (mInstance == NULL) {
            mInstance = new Callbacks();
        }
        return mInstance;
    }
    virtual ~Callbacks();

public:

    void setCallbacks(camera_notify_callback notify_cb,
            camera_data_callback data_cb);
    void takePictureDone();

private:
    camera_notify_callback mNotifyCB;
    camera_data_callback mDataCB;
};

#endif
