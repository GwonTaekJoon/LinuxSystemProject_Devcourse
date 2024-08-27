#ifndef _CALLBACKS_THREAD_H_
#define _CALLBACKS_THREAD_H_

#include <iostream>
#include <cstdio>
#include <unistd.h>

#include <Thread.h>
#include <Vector.h>
#include <MessageQueue.h>

#include "camera_HAL_toy.h"

using std::string;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;

class Callbacks;

class CallbackThread : public Thread {

private:
    static CallbackThread* mInstance;
    CallbackThread();
// constructor destructor
public:
    static CallbackThread* getInstance() {
        if (mInstance == NULL) {
            mInstance = new CallbackThread();
        }
        return mInstance;
    }
    virtual ~CallbackThread();

// public methods
public:
    /* Thread function */
    status_t requestExitAndWait();
    status_t takePictureDone();

// private types
private:

    // thread message id's
    enum MessageId {
        MESSAGE_ID_EXIT = 0,
        MESSAGE_TAKE_PICTURE_DONE,
        // max number of messages
        MESSAGE_ID_MAX
    };

    //
    // message data structures
    //
    struct MessageMsg {
        char *msg;
    };

    // union of all message data
    union MessageData {
        // MESSAGE_MESSAGE,
        MessageMsg msg;
        int data;
    };

    // message id and message data
    struct Message {
        MessageId id;
        MessageData data;
    };

// private methods
private:

    status_t handleMessageExit();
    void handleTakePictureDone();
    status_t waitForAndExecuteMessage();

// inherited from Thread
private:
    virtual void threadLoop();

// private data
private:
    uint32_t mTid;
    MessageQueue<Message, MessageId> mMessageQueue;
    Callbacks *mCallbacks;

// public data
public:

};

#endif
