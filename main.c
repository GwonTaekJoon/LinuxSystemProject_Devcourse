#include <stdio.h>
#include <sys/wait.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>
#include <string.h>
#include <assert.h>
#include <mqueue.h>
#include <toy_message.h>

#define NUM_MESSAGES 10

static mqd_t watchdog_queue;
static mqd_t monitor_queue;
static mqd_t disk_queue;
static mqd_t camera_queue;
static mqd_t engine_queue;
static mqd_t backend_queue;

/* 여기서 sigchld 핸들러 구현 */
static void
sigchldHandler(int sig)
{
    int status;
     int tmp_errno = errno;
     pid_t childPid;

    /*errno is a global variable provided by the standard library of language C,
     indicating the cause of errors when system calls and library functions fail.*/

    while ((childPid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("childPid = %d terminated\n", childPid);
    }

    /*All child processes are terminated */

    errno = tmp_errno;
    //restored errno
}

int create_message_queue(mqd_t *msgq_ptr, const char *queue_name,\
		int num_messages, int message_size)
{

    struct mq_attr mq_attrib;
    int mq_errno;
    mqd_t msgq;

    printf("%s name=%s nummsgs= %d\n", __func__, queue_name, num_messages);
    memset(&mq_attrib, 0, sizeof(mq_attrib));
    mq_attrib.mq_msgsize = message_size;
    mq_attrib.mq_maxmsg = num_messages;

    mq_unlink(queue_name);

    msgq = mq_open(queue_name, O_RDWR | O_CREAT | O_CLOEXEC, 0777, &mq_attrib);

    if(msgq == -1) {

	    printf("%s queue = %s already exists so try to open\n", __func__,\
			    queue_name);

	    msgq = mq_open(queue_name, O_RDWR);
	    assert(msgq != (mqd_t) -1);
	    printf("%s queue=%s opened successfully\n", __func__, queue_name);

	    return -1;
    }



    *msgq_ptr = msgq;
    return 0;

}

int main()
{
    pid_t spid;
    pid_t gpid;
    pid_t ipid;
    pid_t wpid;

    int status;
    int retcode;

    struct sigaction sa;

    memset(&sa, 0, sizeof(sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigchldHandler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        fprintf(stderr,"signal init error");
        return 1;
    }

    printf("main()...\n");

    retcode = create_message_queue(&watchdog_queue, "/watchdog_queue", \
		    NUM_MESSAGES, sizeof(toy_msg_t));
    assert(retcode == 0);

    retcode = create_message_queue(&monitor_queue, "/monitor_queue", \
		    NUM_MESSAGES, sizeof(toy_msg_t));
    assert(retcode == 0);

    retcode = create_message_queue(&disk_queue, "/disk_queue", \
		    NUM_MESSAGES, sizeof(toy_msg_t));
    assert(retcode == 0);

    retcode = create_message_queue(&camera_queue, "/camera_queue", \
		    NUM_MESSAGES, sizeof(toy_msg_t));
    assert(retcode == 0);
    retcode = create_message_queue(&engine_queue, "/engine_queue", \
	        NUM_MESSAGES, sizeof(toy_msg_t));
    assert(retcode == 0);
    retcode = create_message_queue(&backend_queue, "/mq_sys_to_be", \
            NUM_MESSAGES, sizeof(robot_message_t));
    assert(retcode == 0);
    printf("create_system server()...\n");
    spid = create_system_server();


    printf("create_web_server()...\n");
    wpid = create_web_server();


    printf("create_input()...\n");
    ipid = create_input();

    printf("create_gui()...\n");
    gpid = create_gui();




    waitpid(spid, &status, 0);
    waitpid(gpid, &status, 0);
    waitpid(ipid, &status, 0);
    waitpid(wpid, &status, 0);




    return 0;
}
