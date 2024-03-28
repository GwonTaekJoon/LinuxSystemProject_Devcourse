#ifndef _CAMERA_HAL_H_
#define _CAMERA_HAL_H_

#ifdef __cplusplus
//prevention of mangling
extern "C" {

#endif

#include <stdint.h>

int toy_camera_open(void);
int toy_camera_take_picture(void);

#ifdef __cplusplus

}

#endif
#endif
