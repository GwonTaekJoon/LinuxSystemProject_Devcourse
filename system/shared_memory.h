#ifndef _SHARED_MEMORY_H
#define _SHRED_MEMORY_H


enum def_shm_key {


    SHM_KEY_BASE = 10,
    SHM_KEY_SENSOR = SHM_KEY_BASE,
    SHM_KEY_MAX

};

typedef struct shm_sensor {


    int temp;
    int press;
    int humidity;

} shm_sensor_t;

extern int shm_id[SHM_KEY_MAX - SHM_KEY_BASE];

void *toy_shm_create(int key, int size);
void *toy_shm_attach(int shmid);
int toy_shm_detach(void *ptr);
int toy_shm_remove(int shmid);
int toy_shm_get_keyid(int key);


#endif
