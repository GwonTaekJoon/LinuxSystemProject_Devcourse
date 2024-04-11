#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>


#include <shared_memory.h>

int shm_id[SHM_KEY_MAX - SHM_KEY_BASE];


void *toy_shm_create(int key, int size)
{


	/*this is api for using shmget and shmat simultaneously
	 and returning attached shared memory address*/
    printf("%s line %d-> key: %d, size: %d\n", __func__, __LINE__, key, size);

    if(key < SHM_KEY_BASE || key >= SHM_KEY_MAX || size <= 0) {
	printf("%s invalid argument-> key: %d, size: %d\n",\
			__func__, key, size);
    }

    int key_offset = key - SHM_KEY_BASE;
    shm_id[key_offset] = shmget((key_t)key, size, 0666 | IPC_CREAT);

    if(shm_id[key_offset] == -1) {
	printf("shmget error: %d, %s\n", errno, strerror(errno));
	return (void *)-1;
    }


    void *ptr;
    ptr = toy_shm_attach(shm_id[key_offset]);
    if(ptr == (void *)-1) {

	printf("shm_attach error\n");
	return (void *)-1;

    }
    return ptr;

}

void *toy_shm_attach(int shmid)
{

    void *ptr;
    if(shmid < 0) {
	return (void *)-1;
    }

    ptr = shmat(shmid, (void *)0, 0);
    if(ptr == (void *)-1) {
	printf("shmat error : %d, %s\n", errno, strerror(errno));
	return (void *)-1;

    }

    return ptr;
}



int toy_shm_detach(void * ptr)
{


    int ret;
    if(ptr == NULL) {
	return -1;
    }

    ret = shmdt((const void *)ptr);
    if(ret < 0) {
	printf("shmdt error : %d, %s\n", errno, strerror(errno));

    }
    return ret;
}


int toy_shm_remove(int shmid)
{

    int ret;
    if(shmid <= 0) {
	return -1;
    }

    ret = shmctl(shmid, IPC_RMID, NULL);
    if(ret < 0) {
	printf("shmctl(RMID) error: %d, %s\n", errno, strerror(errno));
    }

    return ret;
}


int toy_shm_get_keyid(int key)
{

    if(key < SHM_KEY_BASE || key >= SHM_KEY_MAX) {
	printf("%s invalid argument -> key: %d\n", __func__, key);
	return -1;
    }

    return shm_id[key - SHM_KEY_BASE];
}


