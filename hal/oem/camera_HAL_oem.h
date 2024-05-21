#ifndef _CAMERA_HAL_H_
#define _CAEMRA_HAL_H_


#ifdef __cplusplus
extern "C" { /*for avoiding name mangling*/
#endif

	#include <stdint.h>

	int oem_caemra_open(void);
	int oem_camera_take_picture(void);
	int oem_camera_dump(void);

#ifdef __cplusplus
}
#endif

#endif
