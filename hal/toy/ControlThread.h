#ifndef _CONTROL_THREAD_H_
#define _CONTROL_THREAD_H_

#include <List_comm.h>
#include <Thread.h>
#include <MessageQueue.h>

#include <hardware.h>
#include "Callbacks.h"
#include "CallbackThread.h"
#include "camera_HAL_toy.h"

class ControlThread : public Thread {

// constructor destructor
public:
    ControlThread();
    ~ControlThread();

    status_t init();
    void deinit();

// public methods
public:
    status_t requestExitAndWait();

    void dump();

    // take picture
    status_t takePicture();

    // message callbacks
    void setCallbacks(camera_notify_callback notify_cb,
                      camera_data_callback data_cb);

// private types
private:
    // thread message id's
    enum MessageId {
        MESSAGE_ID_EXIT = 0,
        MESSAGE_ID_TAKE_PICTURE,
        MESSAGE_ID_MAX
    };

    //
    // message data structures
    //
    struct MessageCommand {
        char *cmd;
        int32_t cmd_type;
    };

    // union of all message data
    union MessageData {
        MessageCommand command;
        int cmd;
    };

    // message id and message data
    struct Message {
        MessageId id;
        MessageData data;
    };

    // thread states
    enum State {
        STATE_NONE,
        STATE_BUSY,
    };

private:
    virtual void threadLoop();
    status_t waitForAndExecuteMessage();
    status_t handleMessageTakePicture();

// private data
private:
    MessageQueue<Message, MessageId> mMessageQueue;
    List<Message> mPostponedMessages;
    State mState;
    Callbacks *mCallbacks;
    CallbackThread *mCallbackThread;
}; // class ControlThread

#endif /* _CONTROL_THREAD_H_ */
