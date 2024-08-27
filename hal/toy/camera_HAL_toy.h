#ifndef _CAMERA_HAL_H_
#define _CAMERA_HAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <hardware.h>

int toy_camera_open(void);
int toy_camera_take_picture(void);
int toy_camera_dump(void);
int toy_camera_set_callbacks(int id, camera_notify_callback notify_cb, \
camera_data_callback data_cb);

#ifdef __cplusplus
}    //extern "C"
#endif    //__cplusplus

#endif    // _CAMERA_HAL_H

