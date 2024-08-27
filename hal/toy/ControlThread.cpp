#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <MessageQueue.h>

#include "CallbackThread.h"
#include "ControlThread.h"

using std::string;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;

#define STR_CONCAT(A,B) A << B << ": "
#define __THREAD_FUNC__ STR_CONCAT("[TOY_CT]",__FUNCTION__)

ControlThread::ControlThread() :
    mMessageQueue("ControlThread", (int) MESSAGE_ID_MAX)
    ,mState(STATE_NONE)
{
    cout << __THREAD_FUNC__ << endl;
}

ControlThread::~ControlThread()
{
    cout << __THREAD_FUNC__ << endl;

    cout << __THREAD_FUNC__ << "done" << endl;
}

status_t ControlThread::init()
{
    status_t status = UNKNOWN_ERROR;

    cout << __THREAD_FUNC__ << endl;

    mCallbacks = Callbacks::getInstance();
    if (mCallbacks == NULL) {
        cout << "error creating Callbacks" << endl;
        goto bail;
    }

    mCallbackThread = CallbackThread::getInstance();
    if (mCallbackThread == NULL) {
        cout << "error creating CallbackThread" << endl;
        goto bail;
    }

    status = mCallbackThread->Start();
    if (status != ERROR_NONE) {
        cout << "Error starting callbacks thread!" << endl;
        goto bail;
    }

    return ERROR_NONE;
bail:
    deinit();

    return status;
}

void ControlThread::deinit()
{
    cout << __THREAD_FUNC__ << endl;

    if (mCallbackThread != NULL) {
        mCallbackThread->requestExitAndWait();
        delete mCallbackThread;
    }
    cout << __THREAD_FUNC__ << __LINE__ << endl;

    if (mCallbacks != NULL) {
        delete mCallbacks;
    }
    cout << __THREAD_FUNC__ << __LINE__ << endl;
}

void ControlThread::dump()
{
    cout << __THREAD_FUNC__ << endl;

    cout << __THREAD_FUNC__ << __LINE__ << endl;
}


void ControlThread::setCallbacks(camera_notify_callback notify_cb,
        camera_data_callback data_cb)
{
    cout << __THREAD_FUNC__ << endl;
    mCallbacks->setCallbacks(notify_cb, data_cb);
}

status_t ControlThread::takePicture()
{
    cout << __THREAD_FUNC__ << endl;

    Message msg;
    memset(&msg, 0x00, sizeof(Message));

    msg.id = MESSAGE_ID_TAKE_PICTURE;

    return mMessageQueue.send(&msg);
}

status_t ControlThread::handleMessageTakePicture()
{
    cout << __THREAD_FUNC__ << endl;

    return mCallbackThread->takePictureDone();
}

status_t ControlThread::waitForAndExecuteMessage()
{
    status_t status = ERROR_NONE;
    Message msg;
    memset(&msg, 0x00, sizeof(Message));
    mMessageQueue.receive(&msg);

    switch (msg.id) {
    case MESSAGE_ID_TAKE_PICTURE:
        status = handleMessageTakePicture();
        break;
    default:
        cout << "Invalid message"  << endl;
        status = BAD_VALUE;
        break;
    };

    if (status != ERROR_NONE)
        cout << "Error handling message: " << msg.id << "(" << status << ")" << endl;

    return status;
}

void ControlThread::threadLoop()
{
    status_t status = ERROR_NONE;
    while (1) {
        switch (mState) {
        case STATE_NONE:
            status = waitForAndExecuteMessage();
            break;
        default:
            break;
        };
    }
}

status_t ControlThread::requestExitAndWait()
{
    cout << __THREAD_FUNC__ << endl;

    Message msg;
    memset(&msg, 0x00, sizeof(Message));
    msg.id = MESSAGE_ID_EXIT;
    mMessageQueue.send(&msg);

    return ERROR_NONE;
}
