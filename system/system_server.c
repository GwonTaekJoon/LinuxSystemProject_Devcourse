#include <stdio.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
#include <mqueue.h>
#include <semaphore.h>
#include <sys/shm.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>
//#include <camera_HAL.h> removing for shared libraries

#include <toy_message.h>
#include <shared_memory.h>

#include <sys/inotify.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/sysmacros.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <dump_state.h>
#include <hardware.h>
#include <Engine.h>

#define CAMERA_TAKE_PICTURE 1

#define SENSOR_DATA 1
#define DUMP_STATE 2
#define TOY_TEST_FS "./fs/"
#define BUF_LEN 1024

#define LEFT_MOTOR 0
#define RIGHT_MOTOR 1

static int toy_timer = 0;
pthread_mutex_t system_loop_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t system_loop_cond = PTHREAD_COND_INITIALIZER;
bool	system_loop_exit = false;

static mqd_t watchdog_queue;
static mqd_t monitor_queue;
static mqd_t disk_queue;
static mqd_t camera_queue;
static mqd_t engine_queue;
static mqd_t backend_queue;

static pthread_mutex_t toy_timer_mutex = PTHREAD_MUTEX_INITIALIZER;
static sem_t global_timer_sem;
static bool global_timer_stopped;

static shm_sensor_t *the_sensor_info = NULL;

void signal_exit(void);
void sighandler_timer() {

    pthread_mutex_lock(&toy_timer_mutex);
    ++toy_timer;
    pthread_mutex_unlock(&toy_timer_mutex);
    /*
    //signal_exit();
    time_t rawTime;
    struct tm* formattedTime;
    rawTime = time(NULL);
    //rawTime has a time value in seconds since January 1, 1970 

    formattedTime = localtime(&rawTime);
    //localtime function turns rawTime into easily recognizable value using tm sturct

    int year = formattedTime -> tm_year + 1900;
    int month = formattedTime -> tm_mon + 1;
    int day = formattedTime -> tm_mday;
    int hour = formattedTime -> tm_hour;
    int min = formattedTime -> tm_min;
    int sec = formattedTime -> tm_sec;

    
    printf("Current Time Info : %d year %d month %d day %d:%d:%d\ntimer_count : %d\n"
    ,year, month, day, hour, min, sec, toy_timer);
    */
   


}
void set_periodic_timer(long sec_delay, long usec_delay)
{
	struct itimerval itimer_val = {
		.it_interval = {.tv_sec = sec_delay, .tv_usec = usec_delay},
		.it_value = { .tv_sec = sec_delay, .tv_usec = usec_delay}
	};

	setitimer(ITIMER_REAL, &itimer_val, (struct itimerval*)0);



}

int posix_sleep_ms(unsigned int timeout_ms)
{
    struct timespec sleep_time;

    sleep_time.tv_sec = timeout_ms / MILLISEC_PER_SECOND;
    sleep_time.tv_nsec = (timeout_ms % MILLISEC_PER_SECOND) * (NANOSEC_PER_USEC * USEC_PER_MILLISEC);

    return nanosleep(&sleep_time, NULL);
}


void *watchdog_thread (void * arg) {
    char *s = arg;
    int mqretcode;
    toy_msg_t msg;
    
    printf("%s",s);

    while(1) {
	mqretcode = (int)mq_receive(watchdog_queue, (void *)&msg, sizeof(toy_msg_t), 0);

	assert(mqretcode >= 0);
	printf("watchdog_thread: message arrived");
	printf("msg.type: %d\n", msg.msg_type);
	printf("msg.param1: %d\n", msg.param1);
	printf("msg.param2: %d\n", msg.param2);
    }

    return 0;

}

static long get_directory_size(char *dirname)
{
    DIR *dir = opendir(dirname);

    struct dirent *dit;
    struct stat st;
    long size = 0;
    long total_size = 0;
    char filePath[1024];
    long dir_size = 0;


    if(dir == 0) {
	perror("opening dir error");
	return 0;

    }

    while((dit = readdir(dir)) != NULL) {

	if(strcmp(dit -> d_name, ".") == 0 || strcmp(dit -> d_name, "..") == 0)
		continue;


	sprintf(filePath, "%s/%s", dirname, dit -> d_name);
	if(lstat(filePath, &st) != 0)
		continue;
	size = st.st_size;

	if(S_ISDIR(st.st_mode) != 0) {

	    dir_size = get_directory_size(filePath);
	    total_size += dir_size;

	} else {
		total_size += size;
	}

    }

    return total_size;

}

void *disk_service_thread(void * arg) {
    char *s = arg;
    //FILE* apipe;
    char buf[1024] __attribute__ ((aligned(8)));

    //char cmd[] = "df -h ./";
    //int mqretcode;
    //toy_msg_t msg;
    int inotifyFd;
    int wd;
    ssize_t numRead;
    char *p;
    struct inotify_event *event;
    char *directory = TOY_TEST_FS;
    int total_size;


    printf("%s",s);
    /*
	Output disk remaining every 10 seconds using popen
	Running shell with popen has performance and
	security issues
	Later this will be improved

     */

    inotifyFd = inotify_init();
    if(inotifyFd == -1){

        perror("initing inotify_FD error");
	return 0;

    }
    wd = inotify_add_watch(inotifyFd, TOY_TEST_FS, IN_CREATE);
    if(wd == -1) {
        perror("watching inotify error");
	return 0;

    }


    while(1) {
	numRead = read(inotifyFd, buf, BUF_LEN);
	if(numRead == 0) {

		perror("reading inotifyFd error = 0");
		return 0;

	}
	if(numRead == -1) {
		perror("reading inotifyFd error = -1");
	return 0;
	}

	for(p = buf; p < buf + numRead; ) {
		event = (struct inotify_event *)p;
		p += sizeof(struct inotify_event) + event -> len;

	}
	total_size = get_directory_size(TOY_TEST_FS);
	printf("directory size : %d\n", total_size);


    }

    return 0;

    /*
    while(1) {

	mqretcode = (int)mq_receive(disk_queue, (void *)&msg, sizeof(toy_msg_t), 0);
	assert(mqretcode >= 0);
	printf("disk_service_thread: message arrived");
	printf("msg.type: %d\n", msg.msg_type);
	printf("msg.param1: %d\n", msg.param1);
	printf("msg.param2: %d\n", msg.param2);


        apipe = popen(cmd, "r");
	while( fgets(buf, 1024, apipe) ) {
		printf("%s",buf);
	}
	pclose(apipe);

    }*/

}
void *monitor_thread(void* arg)
{
    char *s = arg;
    int mqretcode;
    toy_msg_t msg;
    robot_message_t robot_msg;
    int shmid;

    printf("%s", s);  // Confirm that the monitor_thread has started
    
    while (1) {
        mqretcode = (int)mq_receive(monitor_queue, (void *)&msg, sizeof(toy_msg_t), 0);
        assert(mqretcode >= 0);

        printf("monitor thread: messages arrived.\n");
        printf("msg.type: %d\n", msg.msg_type);
        printf("msg.param1: %d\n", msg.param1);
        printf("msg.param2: %d\n", msg.param2);
        if(msg.msg_type == SENSOR_DATA) {
            shmid = msg.param1;
            the_sensor_info = toy_shm_attach(shmid);
            printf("sensor temp: %d\n", the_sensor_info -> temp);
            printf("sensor info: %d\n", the_sensor_info -> press);
            printf("sensor humidity %d\n", the_sensor_info -> humidity);

            robot_msg.id = MESSAGE_ID_INFO;
            robot_msg.data.info.id = 12341234;
            robot_msg.data.info.temperature = (int)(the_sensor_info -> temp / 100);
            mqretcode = mq_send(backend_queue, (char *)&robot_msg, sizeof(robot_msg), 0);
            assert(mqretcode == 0);
            toy_shm_detach(the_sensor_info);
        } else if(msg.msg_type == DUMP_STATE) {
            dump_state();
        } else {
            printf("monitor_thread: unknown message. xxx\n");
        }
    }

    return 0;
}

void toy_camera_notify_callback(int32_t msg_type, int32_t ext1, int32_t ext2)
{
    printf("camera_notify_callback: msg_type: %d, exit1: %d, ext2: %d \n", \
    msg_type, ext1, ext2);
}

void toy_camera_data_callback(int32_t msg_type, const camera_data_t *data, \
                             unsigned int index)
{
    printf("camera_data_callback: msg_type: %d, index: %d  \n", msg_type, index);
}

void *camera_service_thread (void * arg) {

    char *s = arg;
    int mqretcode;
    toy_msg_t msg;
    hw_module_t *module = NULL;
    int ret;

    printf("%s",s);


    ret = hw_get_camera_module((const hw_module_t **)&module);
    assert(ret == 0);

    /*
    module -> take_picture(); //test before receiving signal
    printf("Camera module name: %s\n", module -> name);
    printf("Camera module tag: %d\n", module -> tag);
    printf("Camera module id: %s\n", module -> id);
    module -> open();
    module -> set_callbacks(toy_camera_data_callback, toy_camera_data_callback);
    */
    //toy_camera_open();
    while(1) {
	mqretcode = (int)mq_receive(camera_queue, (void *)&msg, sizeof(toy_msg_t), 0);
	assert(mqretcode >= 0);
	printf("camera_service_thread: message arrived");
	printf("msg.type: %d\n", msg.msg_type);
	printf("msg.param1: %d\n", msg.param1);
	printf("msg.param2: %d\n", msg.param2);


	if(msg.msg_type == CAMERA_TAKE_PICTURE) {
	    //toy_camera_take_picture();
	    module -> take_picture();
	} else if(msg.msg_type == DUMP_STATE) {
		//toy_camera_dump();
		module -> dump();
	} else {
		printf("camera_service_thrad: unknown message ...\n");
	}


    }
    return 0;

}

static void timer_expire_signal_handler()
{

	// In signal context, use async-signal-safe function
	//sem_post is async_signal_safe function

    sem_post(&global_timer_sem);
}

static void *timer_thread(void *not_used)
{


    struct sigaction sa;
    memset(&sa, 0, sizeof(sigaction));
    sigemptyset(&sa.sa_mask);
    //safe init

    sa.sa_flags = 0;
    sa.sa_handler = timer_expire_signal_handler;

    if(sigaction(SIGALRM, &sa, NULL) == -1) {

        fprintf(stderr, "timer signal init error");

    }

    /* init timer */
    set_periodic_timer(1, 0);


    while(!global_timer_stopped) {

	int rc = sem_wait(&global_timer_sem);
	if(rc == -1 && errno == EINTR) {
		continue;
	}
	if(rc == -1) {
		perror("sem_wait");
		exit(-1);
	}

	sighandler_timer();
    }

    return 0;
}


void *engine_thread(void *arg)
{
    char *s = arg;
    int mqretcode;
    robot_message_t msg;
    int res;

    printf("%s", s);

    while(1) {
        mqretcode = (int)mq_receive(engine_queue, (void *)&msg, sizeof(robot_message_t), 0);
        assert(mqretcode >= 0);
        printf("engine thread : messages arrived.\n");
        printf("msg.id: %d\n", msg.id);
        printf("msg.data.motor.id: %d\n", msg.data.motor.id);
        printf("msg.data.motor.speed: %d\n", msg.data.motor.speed);
        
        if(msg.id == MESSAGE_ID_ENGINE && msg.data.motor.id == LEFT_MOTOR) { 
            // Left motor
            if(msg.data.motor.speed == 0) { // halt
                halt_left_motor();
            } else {
                set_left_motor_speed(msg.data.motor.speed);
            }
        } else if(msg.id == MESSAGE_ID_ENGINE && msg.data.motor.id == RIGHT_MOTOR) {
            // Right motor
            if(msg.data.motor.speed == 0) { // halt
                halt_right_motor();
            } else {
                set_right_motor_speed(msg.data.motor.speed);
            }
        } else {
            printf("engine thread: unknown message. xxx\n");
        }
    }

    return 0;

}


void signal_exit(void) {

	pthread_mutex_lock(&system_loop_mutex);
	printf("loop exit by signal");
	system_loop_exit = true;
	pthread_cond_signal(&system_loop_cond);
	pthread_mutex_unlock(&system_loop_mutex);

}

int system_server()
{

    pthread_t watchdog_thread_tid;
    pthread_t monitor_thread_tid;
    pthread_t disk_service_thread_tid;
    pthread_t camera_service_thread_tid;
    pthread_t timer_thread_tid;
    
    //real-time
    pthread_t engine_thread_tid;
    pthread_attr_t attr;
    struct sched_param sched_param;
    char short_thread_name[] = "un-named";

    int retcode;

    /* open message queue */

    watchdog_queue = mq_open("/watchdog_queue",O_RDWR);
    assert(watchdog_queue != -1);
    monitor_queue = mq_open("/monitor_queue", O_RDWR);
    assert(monitor_queue != -1);
    disk_queue = mq_open("/disk_queue", O_RDWR);
    assert(disk_queue != -1);
    camera_queue = mq_open("/camera_queue", O_RDWR);
    assert(camera_queue != -1);
    engine_queue = mq_open("/mq_be_to_sys", O_RDWR);
    assert(engine_queue != -1);
    backend_queue = mq_open("/mq_sys_to_be", O_RDWR);
    assert(backend_queue != -1);


    if(pthread_create(&watchdog_thread_tid, NULL, watchdog_thread, "watchdog thread\n") == -1) {
        fprintf(stderr,"pthread_create - watchdog\n");
    }
    if(pthread_create(&disk_service_thread_tid, NULL, disk_service_thread, "disk service thread\n") == -1) {
        fprintf(stderr,"pthread_create - disk_service\n");
    }
    if(pthread_create(&monitor_thread_tid, NULL, monitor_thread, "monitor_thread\n") == -1) {
        fprintf(stderr,"pthread_create - monitor\n");
    }
    if(pthread_create(&camera_service_thread_tid, NULL, camera_service_thread, "camera thread\n") == -1) {
        fprintf(stderr,"pthread_create - camera_service\n");
    }
    if(pthread_create(&timer_thread_tid, NULL, timer_thread, "timer_thread") == -1) {
	fprintf(stderr, "pthread_create - timer_thread\n");
    }


    /* engine thread - real-time class */
    retcode = pthread_attr_init(&attr);
    assert(retcode == 0);
    retcode = pthread_attr_setschedpolicy(&attr, SCHED_RR);
    assert(retcode == 0);
    retcode = pthread_attr_getschedparam(&attr, &sched_param);
    assert(retcode == 0);
    sched_param.sched_priority = 50;
    retcode = pthread_attr_setschedparam(&attr, &sched_param);
    assert(retcode == 0);
    retcode = pthread_create(&engine_thread_tid, &attr, engine_thread, "engine_thread\n");
    assert(retcode == 0);


    
    /*
    pthread_detach(watchdog_thread_tid);
    pthread_detach(disk_service_thread_tid);
    pthread_detach(monitor_thread_tid);
    pthread_detach(camera_service_thread_tid);
    pthread_detach(timer_thread_tid);
    */

   printf("system init done. waiting...\n");

   pthread_mutex_lock(&system_loop_mutex);


   while(system_loop_exit == false) {
	    pthread_cond_wait(&system_loop_cond, &system_loop_mutex);
   }


   pthread_mutex_unlock(&system_loop_mutex);

   printf("<==== system\n");

   while(1) {
	sleep(1);

   }


    return 0;
}

int create_system_server()
{
    pid_t system_server_Pid;
    const char *processName = "system_server";

    printf("creating system_server...\n");

    switch (system_server_Pid = fork())
    {

    case -1:
        printf("system_server fork failed\n");
        break;
    case 0:
        if(prctl(PR_SET_NAME, (unsigned long)processName, NULL, NULL, NULL) == 0) {
            /* man prctl -> int prctl(int option, unsinged long arg2, unsigned long arg2,
             unsigned long arg3, unsigned long arg4, unsigned long arg5)*/

            system_server();
            break;
        }

        perror("prctl()");
        break;

    default:
        break;
    }




    return 0;
}
