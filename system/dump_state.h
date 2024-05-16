#ifndef _DUMP_STATE_H
#define _DUMP_STATE_H

#define KLOG CLOSE 0
#define KLOG_OPEN 1
#define KLOG_READ 2
#define KLOG_READ_ALL 3
#define KLOG_READ_CLEAR 4
#define KLOG_CLEAR 5
#define KLOG_CONSOLE_OFF 6
#define KLOG_CONSOLE_ON 7
#define KLOG_CONSOLE_LEVEL 8
#define KLOG_SIZE_UNREAD 9
#define KLOG_SIZE_BUFFER 10

#define KLOG_DISABLE 6
#define KLOG_ENABLE 7
#define KLOG_SETLEVEL 8
#define KLOG_UNREADSIZE 9
#define KLOG_WRITE 10

void dump_state();

#endif /* _DUMP_STATE_H */

