#include <stdio.h>
#include <sys/prctl.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>
#include <sys/types.h>
#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>
#include <sys/wait.h>
#include <pthread.h>
#include <assert.h>
#include <mqueue.h>
#include <toy_message.h>
#include <shared_memory.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dump_state.h>
#include <seccomp.h>

#define TOY_TOK_BUFSIZE 64
#define TOY_TOK_DELIM " \t\r\n\a"
#define TOY_BUFFSIZE 1024

#define DUMP_STATE 2


typedef struct _sig_ucontext {
    unsigned long uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    struct sigcontext uc_mcontext;
    sigset_t uc_sigmask;
} sig_ucontext_t;

static pthread_mutex_t global_message_mutex = PTHREAD_MUTEX_INITIALIZER;

static char global_message[TOY_BUFFSIZE];
// Warning global variable - global_message => mutex

static mqd_t watchdog_queue;
static mqd_t monitor_queue;
static mqd_t disk_queue;
static mqd_t camera_queue;

static shm_sensor_t *the_sensor_info = NULL;


void segfault_handler(int sig_num, siginfo_t * info, void * ucontext) {
  void * array[50];
  void * caller_address;
  char ** messages;
  int size, i;
  sig_ucontext_t * uc;

  uc = (sig_ucontext_t *) ucontext;

  /* Get the address at the time the signal was raised */
  caller_address = (void *) uc->uc_mcontext.pc;  // rip: x86_64 specific     arm_pc: pc

  fprintf(stderr, "\n");

  if (sig_num == SIGSEGV)
    printf("signal %d (%s), address is %p from %p\n", sig_num, strsignal(sig_num), info->si_addr,
           (void *) caller_address);
  else
    printf("signal %d (%s)\n", sig_num, strsignal(sig_num));

  size = backtrace(array, 50);
  /* overwrite sigaction with caller's address */
  array[1] = caller_address;
  messages = backtrace_symbols(array, size);

  /* skip first stack frame (points here) */
  for (i = 1; i < size && messages != NULL; ++i) {
    printf("[bt]: (%d) %s\n", i, messages[i]);
  }

  free(messages);

  exit(EXIT_FAILURE);
}

/*
 *  sensor thread
 */
void *sensor_thread(void* arg)
{

    int mqretcode;
    char *s = arg;
    toy_msg_t msg;
    int shmid = toy_shm_get_keyid(SHM_KEY_SENSOR);

    printf("%s",s);

    while(1) {

	posix_sleep_ms(5000);

	/*save current sensor information on system V shared memory
	send messages to monitor thread
	 */

	if(the_sensor_info != NULL) {

		/*before attaching real sensor, hard coding*/
	    the_sensor_info -> temp = 35;
	    the_sensor_info -> press = 55;
	    the_sensor_info -> humidity = 80;

	}

	msg.msg_type = 1;
	msg.param1 = shmid;
	msg.param2 = 0;
	mqretcode = mq_send(monitor_queue, (char *)&msg, sizeof(msg), 0);
	assert(mqretcode == 0);

    }
    return 0;
}

int toy_send(char **args);
int toy_shell(char **args);
int toy_exit(char **args);
int toy_mutex(char **args);
int toy_message_queue(char **args);
int toy_read_elf_header(char **args);
int toy_dump_state(char **args);
int toy_mincore(char **args);
int toy_simple_io(char **args);

char *builtin_str[] = {
    "send",
    "sh",
    "exit",
    "mu",
    "mq",
    "elf",
    "dump",
    "mincore",
    "n"
};

int (*builtin_func[]) (char **) = {
    &toy_send,
    &toy_shell,
    &toy_exit,
    &toy_mutex,
    &toy_message_queue,
    &toy_read_elf_header,
    &toy_dump_state,
    &toy_mincore,
    &toy_simple_io
};

int toy_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

int toy_send(char **args)
{
    printf("send message: %s\n", args[1]);

    return 1;
}

int toy_mutex(char **args)
{

	if(args[1] == NULL) {

		return 1;
	}

	printf("save message : %s\n", args[1]);
	//require mutex
	pthread_mutex_lock(&global_message_mutex);
	strcpy(global_message, args[1]);
	pthread_mutex_unlock(&global_message_mutex);

	return 1;

}


int toy_message_queue(char **args)
{

    int mqretcode;
    toy_msg_t msg;

    if(args[1] == NULL || args[2] == NULL) {
	    return 1;
    }

    if(!strcmp(args[1], "camera")) {
	msg.msg_type = atoi(args[2]);
	msg.param1 = 0;
	msg.param2 = 0;
	mqretcode = mq_send(camera_queue, (char *)&msg, sizeof(msg), 0);
	assert(mqretcode == 0);

    }

    return 1;


}

int toy_read_elf_header(char **args)
{

    int mqretcode;
    toy_msg_t msg;
    int in_fd;
    char *contents = NULL;
    size_t contents_sz;
    struct stat st;
    Elf64Hdr *map;

    in_fd = open("./sample/sample.elf", O_RDONLY);
    if(in_fd < 0) {
	perror("cannot open ./sample/sample.elf");
	return EXIT_FAILURE;
    }

    if(fstat(in_fd, &st) == -1) {
	perror("fstat error");
	close(in_fd);
	return EXIT_FAILURE;
    }

    contents_sz = st.st_size;
    if(contents_sz == 0) {
	printf("./sample/sample.elf is empty\n");
	return 1;
    }
    printf("real size: %ld", contents_sz);
        map = (Elf64Hdr *)mmap(NULL, contents_sz, PROT_READ,\
			MAP_PRIVATE, in_fd, 0);
    printf("Object file type : %d", map->e_type);
    printf("Architecture : %d", map->e_machine);
    printf("Object file version : %d", map->e_version);
    printf("Entry point virtual address : %ld", map->e_entry);
    printf("Program header table file offset : %ld", map->e_phoff);
    munmap(map, contents_sz);
    close(in_fd);
    return 1;
}
int toy_mincore(char **args)
{

    unsigned char vec[20];
    int res;
    size_t page = sysconf(_SC_PAGESIZE);
    void *addr = mmap(NULL, 20 * page, PROT_READ | PROT_WRITE, \
		    MAP_NORESERVE | MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    res = mincore(addr, 10 * page, vec);
    assert(res == 0);

    return 1;


}
int toy_exit(char **args)
{
    return 0;
}

int toy_shell(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("toy");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("toy");
    } else {    
        do
        {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int toy_execute(char **args)
{
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < toy_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return 1;
}

char *toy_read_line(void)
{
    char *line = NULL;
    ssize_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        } else {
            perror(": getline\n");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

char **toy_split_line(char *line)
{
    int bufsize = TOY_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token, **tokens_backup;

    if (!tokens) {
        fprintf(stderr, "toy: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOY_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += TOY_TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                free(tokens_backup);
                fprintf(stderr, "toy: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, TOY_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

void toy_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf("TOY>");
        line = toy_read_line();
        args = toy_split_line(line);
        status = toy_execute(args);

        free(line);
        free(args);
    } while (status);
}

int toy_dump_state(char **args)
{

    int mqretcode;
    toy_msg_t msg;

    msg.msg_type = DUMP_STATE;
    msg.param1 = 0;
    msg.param2 = 0;

    mqretcode = mq_send(camera_queue, (char *)&msg, sizeof(msg), 0);
    assert(mqretcode == 0);


    mqretcode = mq_send(monitor_queue, (char *)&msg, sizeof(msg), 0);
    assert(mqretcode == 0);

    return 1;


}

int toy_simple_io(char **args) 
{

    int dev;
    char buff = 2;
    dev = open("/dev/toy_simple_io_driver", O_RDWR | O_NDELAY);

    if(dev < 0) {
        fprintf(stderr, "module open error");
        return 1;    
    }

    if(args[1] != NULL && !strcmp(args[1], "exit")) {
        buff = 2;
        if(write(dev, &buff, 1) < 0) {
            printf("write error\n");
            goto err;
        }
    } else {
        buff = 1;
        if(write(dev, &buff, 1) < 0) {
            printf("write error\n");
            goto err;
        }
    }

    

err:
    close(dev);


    return 1;


}

void *command_thread(void* arg)
{
    char *s = arg;

    printf("%s", s);

    toy_loop();

    return 0;
}


int input()
{
    printf("input()...\n");

    pthread_t command_thread_tid;
    pthread_t sensor_thread_tid;




    /*  signal */

    struct sigaction sa;

    memset(&sa, 0, sizeof(sigaction));
    sigemptyset(&sa.sa_mask);
    //safe init

    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sa.sa_sigaction = segfault_handler;

    if(sigaction(SIGSEGV, &sa, NULL) == -1) {

        fprintf(stderr,"signal init error");
        exit(1);

    }

    scmp_filter_ctx ctx;
    ctx = seccomp_init(SCMP_ACT_ALLOW);
    if(ctx == NULL) {
        printf("seccomp_init failed");
	return -1;
    }

    /* Invoking a "mincore" system call induces an error */
    int rc = seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(mincore), 0);
    if(rc < 0) {
        printf("seccomp_rule_add failed");
	return -1;

    }

    /* creating filter log*/
    seccomp_export_pfc(ctx, 5);
    seccomp_export_bpf(ctx, 6);


    rc = seccomp_load(ctx);

    if(rc < 0) {
        printf("seccomp_load failed");
	return -1;
    }

    /*seccomp filter drive*/
    seccomp_release(ctx);












	/* for sharing sensor data, create system V shared memory */
    the_sensor_info = (shm_sensor_t *)toy_shm_create(SHM_KEY_SENSOR,\
sizeof(shm_sensor_t));
    if(the_sensor_info == (void *) -1) {

	    the_sensor_info = NULL;
	    printf("Error in shm_create SHMID = %d SHM_KEY_SENSOR\n", \
			    SHM_KEY_SENSOR);


    }




    /* open message queue */
    watchdog_queue = mq_open("/watchdog_queue", O_RDWR);
    assert(watchdog_queue != -1);
    monitor_queue = mq_open("/monitor_queue", O_RDWR);
    assert(monitor_queue != -1);
    disk_queue = mq_open("/disk_queue", O_RDWR);
    assert(disk_queue != -1);
    camera_queue = mq_open("/camera_queue", O_RDWR);
    assert(camera_queue != -1);





    /* create threads */

    if(pthread_create(&command_thread_tid, NULL, command_thread, "command_thread...\n") == -1) {
        fprintf(stderr, "pthread_create error - command_thread");
    }

    if(pthread_create(&sensor_thread_tid, NULL, sensor_thread, "sensor_thread...\n") == -1) {
        fprintf(stderr, "pthread_create error - sensor_thread");
    }


    //pthread_detach(command_thread_tid);
    //pthread_detach(sensor_thread_tid);




    while (1) {
        sleep(1);
    }

    return 0;
}

int create_input()
{
    pid_t inputPid;
    const char *processName = "input";

    printf("creating input...\n");

    switch (inputPid = fork())
    {
    case -1:
        printf("input fork failed\n");
        break;

    case 0:
        if(prctl(PR_SET_NAME, (unsigned long)processName, NULL, NULL, NULL) < 0){
            /* man prctl -> int prctl(int option, unsinged long arg2, unsigned long arg2,
             unsigned long arg3, unsigned long arg4, unsigned long arg5)*/
            perror("prctl()");

        }
        input();
        break;
    default:
        break;
    }

    return 0;
}
