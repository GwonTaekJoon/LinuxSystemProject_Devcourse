#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h> // <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/delay.h>

#define BUF_SIZE    1024

/* left leds */

#define TOY_GPIO_OUTPUT_LEFT_1 17
#define TOY_GPIO_OUTPUT_LEFT_2 27
#define TOY_GPIO_OUTPUT_LEFT_3 22
#define TOY_GPIO_OUTPUT_LEFT_4 5

/* right leds */

#define TOY_GPIO_OUTPUT_RIGHT_1 6
#define TOY_GPIO_OUTPUT_RIGHT_2 26
#define TOY_GPIO_OUTPUT_RIGHT_3 23
#define TOY_GPIO_OUTPUT_RIGHT_4 24

/* switch */

#define TOY_GPIO_INPUT 16 

#define MAX_TIMEOUT_MSEC    2000

#define MOTOR_1_SET_SPEED _IOW('w', '1', int32_t *)
#define MOTOR_2_SET_SPEED _IOW('w', '2', int32_t *)

#define DRIVER_NAME "toy_engine_driver"
#define DRIVER_CLASS "toy_engine_class"

#define ON 1
#define OFF 0


#define MOTOR_1_LED_LEFT_ON()    \
do {    \
    gpio_set_value(TOY_GPIO_OUTPUT_LEFT_1, 1);    \
    gpio_set_value(TOY_GPIO_OUTPUT_LEFT_2, 1);    \
} while(0)

#define MOTOR_1_LED_LEFT_OFF()    \
do {    \
    gpio_set_value(TOY_GPIO_OUTPUT_LEFT_1, 0);    \
    gpio_set_value(TOY_GPIO_OUTPUT_LEFT_2, 0);    \
} while(0)

#define MOTOR_1_LED_RIGHT_ON()    \
do {    \
    gpio_set_value(TOY_GPIO_OUTPUT_LEFT_3, 1);    \
    gpio_set_value(TOY_GPIO_OUTPUT_LEFT_4, 1);    \
} while(0)

#define MOTOR_1_LED_RIGHT_OFF()    \
do {    \
    gpio_set_value(TOY_GPIO_OUTPUT_LEFT_3, 0);    \
    gpio_set_value(TOY_GPIO_OUTPUT_LEFT_4, 0);    \
} while(0)

#define MOTOR_2_LED_LEFT_ON()    \
do {    \
    gpio_set_value(TOY_GPIO_OUTPUT_RIGHT_1, 1);    \
    gpio_set_value(TOY_GPIO_OUTPUT_RIGHT_2, 1);    \
} while(0)


#define MOTOR_2_LED_LEFT_OFF()    \
do {    \
    gpio_set_value(TOY_GPIO_OUTPUT_RIGHT_1, 0);    \
    gpio_set_value(TOY_GPIO_OUTPUT_RIGHT_2, 0);    \
} while(0)


#define MOTOR_2_LED_RIGHT_ON()     \
do {    \
    gpio_set_value(TOY_GPIO_OUTPUT_RIGHT_3, 1);    \
    gpio_set_value(TOY_GPIO_OUTPUT_RIGHT_4, 1);    \
} while(0)

#define MOTOR_2_LED_RIGHT_OFF()    \
do {    \
    gpio_set_value(TOY_GPIO_OUTPUT_RIGHT_3, 0);    \
    gpio_set_value(TOY_GPIO_OUTPUT_RIGHT_4, 0);    \
} while(0)



static dev_t toy_dev;
static struct class *toy_class;
static struct cdev toy_device;
static unsigned int button_irq;

static struct hrtimer motor_1_hrtimer;
static struct hrtimer motor_2_hrtimer;

int motor_1_toggle;
int motor_2_toggle;
static int motor_1_speed = 1;
static int motor_2_speed = 1;

static int register_gpio_output(int gpio_num);

static irq_handler_t k_gpio_irq_signal_handler(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
    struct siginfo info;

    pr_info("Emergency stop!!\n");

    hrtimer_try_to_cancel(&motor_1_hrtimer);
    hrtimer_try_to_cancel(&motor_2_hrtimer);

    MOTOR_1_LED_LEFT_OFF();
    MOTOR_1_LED_RIGHT_OFF();
    
    MOTOR_2_LED_LEFT_OFF();
    MOTOR_2_LED_RIGHT_OFF();

    return (irq_handler_t) IRQ_HANDLED;
}

/* 0 = HRTIMER_NORESTART, 1 = HRTIMER_RESTART */
enum hrtimer_restart motor_1_timer_callback(struct hrtimer *timer)
{
    pr_info("motor 1 callback function : jiffies value - (%ld).\n", jiffies);

    motor_1_toggle = !motor_1_toggle;
    if(motor_1_toggle) {
        MOTOR_1_LED_LEFT_ON();
        MOTOR_1_LED_RIGHT_OFF();
    } else {
        MOTOR_1_LED_LEFT_OFF();
        MOTOR_1_LED_RIGHT_ON();
    }
    hrtimer_forward_now(timer, ms_to_ktime(MAX_TIMEOUT_MSEC - \
    (MAX_TIMEOUT_MSEC * motor_1_speed / 100 + 1)));
    return HRTIMER_RESTART;
}

enum hrtimer_restart motor_2_timer_callback(struct hrtimer *timer)
{

    pr_info("motor 2 callback function: jiffies value - %(ld).\n", jiffies);

    motor_2_toggle = !motor_2_toggle;
    if(motor_2_toggle) {
        MOTOR_2_LED_LEFT_ON();
        MOTOR_2_LED_RIGHT_OFF();
    } else {
            MOTOR_2_LED_LEFT_OFF();
            MOTOR_2_LED_RIGHT_ON();
    }

    hrtimer_forward_now(timer, ms_to_ktime(MAX_TIMEOUT_MSEC - \
    (MAX_TIMEOUT_MSEC * motor_2_speed / 100 + 1)));
    return HRTIMER_RESTART; 
}

static ssize_t toy_driver_read(struct file *flip, char __user *buf, size_t count, loff_t *offset)
{
    return BUF_SIZE;
}

ssize_t toy_driver_write(struct file *flip, const char __user *buf, size_t count, loff_t *offset)
{
    pr_info("write: done\n");

    return count;
}

static int toy_driver_open(struct inode *device_file, struct file *instance)
{
    pr_info("open\n");
    return 0;
}

static int toy_driver_close(struct inode *device_file, struct file *instance)
{
    pr_info("close\n");
    return 0;
}

static long int toy_ioctl(struct file *file, unsigned cmd, unsigned long arg)
{
    
    switch (cmd)
    {
    case MOTOR_1_SET_SPEED:
        pr_info("MOTOR_1: %d\n", *(int *)arg);
        motor_1_speed = *(int *)arg;
        break;
    case MOTOR_2_SET_SPEED:
        pr_info("MOTOR_2: %d\n", *(int *)arg);
        motor_2_speed = *(int *)arg;
        break;
    
    default:
        break;
    }

    return EINVAL;
}


static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = toy_driver_open,
    .release = toy_driver_close,
    .read = toy_driver_read,
    .write = toy_driver_write,
    .unlocked_ioctl = toy_ioctl
};

static int register_gpio_output(int gpio_num)
{

    char name[80];

    snprintf(name, sizeof(name), "toy-gpio-%d", gpio_num);

    if(gpio_request(gpio_num, name)) {
        pr_info("Can not allocate GPIO \n");
        return -1;
    }

    if(gpio_direction_output(gpio_num, 0)) {
        pr_info("Can not set GPIO to output!\n");
        return -1;
    }

    return 0;
}


static int __init toy_module_init(void)
{

    if(alloc_chrdev_region(&toy_dev, 0, 1, DRIVER_NAME) < 0) {
        pr_info("Device \n");
        return -1;
    }

    if((toy_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
        pr_info("Device class can not be created!\n");
        goto cerror;
    }

    if(device_create(toy_class, NULL, toy_dev, NULL, DRIVER_NAME) == NULL) {
        pr_info("Can not create device file!\n");
        goto device_error;
    }

    cdev_init(&toy_device, &fops);

    if(cdev_add(&toy_device, toy_dev, 1) == -1) {
        pr_info("Register of device to kernel failed!\n");
        goto reg_error;
    }


    register_gpio_output(TOY_GPIO_OUTPUT_LEFT_1);
    register_gpio_output(TOY_GPIO_OUTPUT_LEFT_2);
    register_gpio_output(TOY_GPIO_OUTPUT_LEFT_3);
    register_gpio_output(TOY_GPIO_OUTPUT_LEFT_4);

    register_gpio_output(TOY_GPIO_OUTPUT_RIGHT_1);
    register_gpio_output(TOY_GPIO_OUTPUT_RIGHT_2);
    register_gpio_output(TOY_GPIO_OUTPUT_RIGHT_3);
    register_gpio_output(TOY_GPIO_OUTPUT_RIGHT_4);

    if(gpio_request(TOY_GPIO_INPUT, "toy-gpio-16")) {
        pr_info("Can not allocate GPIO 16!\n");
        goto gpio_error;
    }

    if(gpio_direction_input(TOY_GPIO_INPUT)) {
        pr_info("Can not set GPIO 16 to input!\n");
        goto gpio_error;
    }

    button_irq = gpio_to_irq(16);

    /* blocking chattering */
    gpio_set_debounce(16, 300);

    if(request_irq(button_irq, (irq_handler_t) k_gpio_irq_signal_handler, IRQF_TRIGGER_RISING,\
    "k_gpio_irq_signal", NULL) != 0) {
        pr_err("Error \n Can not request interrupt nr: %d\n", button_irq);
        goto gpio_error;
    }

    hrtimer_init(&motor_1_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    motor_1_hrtimer.function = &motor_1_timer_callback;
    hrtimer_start(&motor_1_hrtimer, ms_to_ktime(MAX_TIMEOUT_MSEC), HRTIMER_MODE_REL);

    hrtimer_init(&motor_2_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    motor_2_hrtimer.function = &motor_2_timer_callback;
    hrtimer_start(&motor_2_hrtimer, ms_to_ktime(MAX_TIMEOUT_MSEC), HRTIMER_MODE_REL);

    return 0;

gpio_error:
    gpio_free(TOY_GPIO_INPUT);
    gpio_free(TOY_GPIO_OUTPUT_LEFT_1);
    gpio_free(TOY_GPIO_OUTPUT_LEFT_2);
    gpio_free(TOY_GPIO_OUTPUT_LEFT_3);
    gpio_free(TOY_GPIO_OUTPUT_LEFT_4);
    gpio_free(TOY_GPIO_OUTPUT_RIGHT_1);
    gpio_free(TOY_GPIO_OUTPUT_RIGHT_2);
    gpio_free(TOY_GPIO_OUTPUT_RIGHT_3);
    gpio_free(TOY_GPIO_OUTPUT_RIGHT_4);

reg_error:
    device_destroy(toy_class, toy_dev);

device_error:
    class_destroy(toy_class);

cerror:
    unregister_chrdev_region(toy_dev, 1);

    return -1;

}


static void __exit toy_module_exit(void)
{

    int retval;

    free_irq(button_irq, NULL);
    retval = hrtimer_cancel(&motor_1_hrtimer);
    if(retval) {
        pr_info("timer delete ...\n");
    }
    retval = hrtimer_cancel(&motor_2_hrtimer);
    if(retval) {
        pr_info("timer delete ...\n");
    }

    gpio_set_value(TOY_GPIO_INPUT, 0);
    gpio_free(TOY_GPIO_INPUT);

    gpio_free(TOY_GPIO_OUTPUT_LEFT_1);
	gpio_free(TOY_GPIO_OUTPUT_LEFT_2);
	gpio_free(TOY_GPIO_OUTPUT_LEFT_3);
	gpio_free(TOY_GPIO_OUTPUT_LEFT_4);
	gpio_free(TOY_GPIO_OUTPUT_RIGHT_1);
	gpio_free(TOY_GPIO_OUTPUT_RIGHT_2);
	gpio_free(TOY_GPIO_OUTPUT_RIGHT_3);
	gpio_free(TOY_GPIO_OUTPUT_RIGHT_4);

    cdev_del(&toy_device);
    device_destroy(toy_class, toy_dev);
    class_destroy(toy_class);
    unregister_chrdev_region(toy_dev, 1);
    pr_info("exit\n");

}

module_init(toy_module_init);
module_exit(toy_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GwonTaekJoon<kyuzu123123@gmail.com>");
MODULE_DESCRIPTION("toy engine module");
MODULE_VERSION("1.0.0");

