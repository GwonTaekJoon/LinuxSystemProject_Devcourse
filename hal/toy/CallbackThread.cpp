#include <iostream>
#include <cstdio>
#include <unistd.h>

#include <hardware.h>

#include "Callbacks.h"
#include "CallbackThread.h"

CallbackThread* CallbackThread::mInstance = NULL;

using std::string;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;

#define STR_CONCAT(A,B) A << B << ": "
#define __THREAD_FUNC__ STR_CONCAT("[TOY_CBT]",__FUNCTION__)

CallbackThread::CallbackThread() :
    mMessageQueue("CallbackThread", MESSAGE_ID_MAX)
    ,mCallbacks(Callbacks::getInstance())
{
    cout << __THREAD_FUNC__ << endl;
}

CallbackThread::~CallbackThread()
{
    cout << __THREAD_FUNC__ << endl;
    mInstance = NULL;
}

status_t CallbackThread::handleMessageExit()
{
    status_t status = ERROR_NONE;

    cout << __THREAD_FUNC__  << endl;

    return status;
}

status_t CallbackThread::takePictureDone(void)
{
    Message msg;
    memset(&msg, 0x00, sizeof(Message));
    msg.id = MESSAGE_TAKE_PICTURE_DONE;

    return mMessageQueue.send(&msg);
}

void CallbackThread::handleTakePictureDone()
{
    mCallbacks->takePictureDone();
}

status_t CallbackThread::waitForAndExecuteMessage()
{
    //cout << __THREAD_FUNC__  << endl;
    status_t status = ERROR_NONE;
    Message msg;
    memset(&msg, 0x00, sizeof(Message));
    mMessageQueue.receive(&msg);

    switch (msg.id) {
    case MESSAGE_TAKE_PICTURE_DONE:
        handleTakePictureDone();
        break;
    default:
        status = BAD_VALUE;
        break;
    };

    return status;
}

void CallbackThread::threadLoop()
{
    cout << __THREAD_FUNC__  << endl;
    status_t status = ERROR_NONE;
    mTid = pthread_self();
    cout << mTid << " << CallbackThread TID" << endl;
    while (1)
        status = waitForAndExecuteMessage();
}

status_t CallbackThread::requestExitAndWait()
{
    cout << __THREAD_FUNC__  << endl;

    Message msg;
    memset(&msg, 0x00, sizeof(Message));
    msg.id = MESSAGE_ID_EXIT;
    mMessageQueue.send(&msg);

    return ERROR_NONE;
}

