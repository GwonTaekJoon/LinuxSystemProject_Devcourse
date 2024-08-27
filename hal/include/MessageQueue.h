
/* This code refers from android framework */

#ifndef MESSAGE_QUEUE
#define MESSAGE_QUEUE

#include <stdio.h>
#include <Errors.h>
#include <List_comm.h>
#include <Vector.h>
#include <Mutex.h>
#include <Condition.h>


template <class MessageType, class MessageId>
class MessageQueue {

    // constructor / destructor
public:
    MessageQueue(const char *name, // for debugging
            int numReply = 0) :    // set numReply only if you need synchronous messages
        mName(name)
        ,mNumReply(numReply)
        ,mReplyMutex(NULL)
        ,mReplyCondition(NULL)
        ,mReplyStatus(NULL)
    {
        if (mNumReply > 0) {
            mReplyMutex = new Mutex[numReply];
            mReplyCondition = new Condition[numReply];
            mReplyStatus = new status_t[numReply];
        }
    }

    ~MessageQueue()
    {
        if (size() > 0) {
            // The last message a thread should receive is EXIT.
            // If for some reason a thread is sent a message after
            // the thread has exited then there is a race condition
            // or design issue.
            printf("~MessageQueue: %s queue (unhandled message:%d) forcefully deleted\n", mName, size());
        }
        else
            printf("~MessageQueue: %s queue (unhandled message:%d) deleted\n", mName, size());

        if (mNumReply > 0) {
            delete [] mReplyMutex;
            delete [] mReplyCondition;
            delete [] mReplyStatus;
        }
    }

    // public methods
public:

    // Push a message onto the queue. If replyId is not -1 function will block until
    // the caller is signalled with a reply. Caller is unblocked when reply method is
    // called with the corresponding message id.
    status_t send(MessageType *msg, MessageId replyId = (MessageId) -1)
    {
        status_t status = ERROR_NONE;

        // someone is misusing the API. replies have not been enabled
        if (replyId != -1 && mNumReply == 0) {
            printf("MessageQueue error: %s replies not enabled\n", mName);
            return BAD_VALUE;
        }

        mQueueMutex.lock();
        MessageType data = *msg;
        mList.push_front(data);
        if (replyId != -1) {
            mReplyStatus[replyId] = WOULD_BLOCK;
        }
        mQueueCondition.signal();
        mQueueMutex.unlock();

        if (replyId >= 0 && status == ERROR_NONE) {
            mReplyMutex[replyId].lock();
            while (mReplyStatus[replyId] == WOULD_BLOCK) {
                mReplyCondition[replyId].wait(mReplyMutex[replyId]);
                // wait() should never complete without a new status having
                // been set, but for diagnostic purposes let's check it.
                if (mReplyStatus[replyId] == WOULD_BLOCK) {
                    printf("MessageQueue - woke with WOULD_BLOCK\n");
                }
            }
            status = mReplyStatus[replyId];
            mReplyMutex[replyId].unlock();
        }

        return status;
    }

    status_t remove(MessageId id, Vector<MessageType> *vect = NULL)
    {
        status_t status = ERROR_NONE;
        if(isEmpty())
            return status;

        mQueueMutex.lock();
        typename List<MessageType>::iterator it = mList.begin();
        while (it != mList.end()) {
            MessageType msg = *it;
            if (msg.id == id) {
                if (vect) {
                    vect->push(msg);
                }
                it = mList.erase(it); // returns pointer to next item in list
            } else {
                it++;
            }
        }
        mQueueMutex.unlock();

        // unblock caller if waiting
        if (mNumReply > 0) {
            reply(id, INVALID_OPERATION);
        }

        return status;
    }

    // Pop a message from the queue
    status_t receive(MessageType *msg)
    {
        status_t status = ERROR_NONE;

        mQueueMutex.lock();
        while (isEmptyLocked()) {
            mQueueCondition.wait(mQueueMutex);
            // wait() should never complete without a message being
            // available, but for diagnostic purposes let's check it.
            if (isEmptyLocked()) {
                printf("MessageQueue - woke with mCount == 0\n");
            }
        }

        *msg = *(--mList.end());
        mList.erase(--mList.end());
        mQueueMutex.unlock();

        return status;
    }

    // Unblock the caller of send and indicate the status of the received message
    void reply(MessageId replyId, status_t status)
    {
        mReplyMutex[replyId].lock();
        mReplyStatus[replyId] = status;
        mReplyCondition[replyId].signal();
        mReplyMutex[replyId].unlock();
    }

    // Return true if the queue is empty
    bool isEmpty() {
        Mutex::Autolock lock(mQueueMutex);
        return sizeLocked() == 0;
    }

    int size() {
        Mutex::Autolock lock(mQueueMutex);
        return sizeLocked();
    }

private:

    // Return true if the queue is empty, must be called
    // with mQueueMutex taken
    inline bool isEmptyLocked() { return sizeLocked() == 0; }

    inline int sizeLocked() { return mList.size(); }

    const char *mName;
    Mutex mQueueMutex;
    Condition mQueueCondition;
    List<MessageType> mList;

    int mNumReply;
    Mutex *mReplyMutex;
    Condition *mReplyCondition;
    status_t *mReplyStatus;

}; // class MessageQueue

#endif // MESSAGE_QUEUE
