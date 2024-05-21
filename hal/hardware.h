#ifndef _HARDWARE_H
#define _HARDWARE_H

#include <stdint.h>

#define HAL_MODULE_INFO_SYM	HMI
#define HAL_MODULE_INFO_SYM_AS_STR	"HMI"


#define MAKE_TAG_CONSTANT(A,B,C,D) (((A) << 24) | ((B) << 16) | ((C) << 8) | (D))
/* MAKE_TAG_CONSTANT is to
 shift to the left in 8-bit increments and combine them into 32-bit integers*/

#define HARDWARE_MODULE_TAG MAKE_TAG_CONSTANT('H', 'W', 'M', 'T')
#define HARDWARE_DEVICE_TAG MAKE_TAG_CONSTANT('H', 'W', 'D', 'T')


#define HARDWARE_MAKE_API_VERSION(maj, min) \
	((((maj) & 0xff) << 8) | ((min) & 0xff))

#define HARDWARE_MAKE_API_VERSION_2 (maj, min, hdr) \
	((((maj) & 0xff) << 24) | (((min) & 0xff) << 16) | ((hdr) & 0xffff))



#define HARDWARE_API_VERSION_2_MAJ_MIN_MASK 0xffff0000
#define HARDWARE_API_VERISON_2_HEADER_MASK 0x0000ffff

#define CAMERA_HARDWARE_MODULE_ID "camera"



typedef struct hw_module_t {

    uint32_t tag;
    const char *id;
    const char *name;
    int (*open)();
    int (*take_picture)();
    int (*dump)();
    void* dso;

} hw_module_t;


int hw_get_camera_module(const struct hw_module_t **module);

#endif

