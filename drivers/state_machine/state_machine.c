#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/fs.h>

#define GPIO_BUTTON 16  // GPIO pin for the button
#define DOUBLE_CLICK_INTERVAL 500  // 0.5 seconds for double-click detection

/* left led */
#define GPIO_OUTPUT_LEFT_1 17
#define GPIO_OUTPUT_LEFT_2 27
#define GPIO_OUTPUT_LEFT_3 22
#define GPIO_OUTPUT_LEFT_4 5

/*right led */
#define GPIO_OUTPUT_RIGHT_1 6
#define GPIO_OUTPUT_RIGHT_2 26
#define GPIO_OUTPUT_RIGHT_3 23
#define GPIO_OUTPUT_RIGHT_4 24

#define DRIVER_NAME "switch_single_double_click_driver"
#define DRIVER_CLASS "toy_simple_io_class"
#define ON 1
#define OFF 0

#define NUM_STATE 7
#define NUM_INPUT 3

#define BUF_SIZE 1024

static char kernel_write_buffer[BUF_SIZE];
static dev_t toy_dev;
static struct class *toy_class;
static struct cdev toy_device;

static struct timer_list click_timer;
static struct timer_list led_on_timers[4];
static struct timer_list led_off_timers[4];


enum { SW_ON, SW_OFF, T_OUT };
enum {ACTION_NOP, ACTION_LEFT_ON, ACTION_LEFT_OFF, ACTION_ALL_ON, ACTION_ALL_OFF,
        ACTION_LEFT_4_ON, ACTION_LEFT_6_ON, ACTION_RESET_TIMER };

struct state_machine_element {
    int next_state;
    int action;
};

/* define state machine */
struct state_machine_element state_machine_table[NUM_STATE][NUM_INPUT] = {
    /* State 0: initial state */
    { {1, ACTION_LEFT_ON}, {0, ACTION_NOP}, {0, ACTION_NOP} },
    /* State 1: short click (Turn on led one by one from the left) */
    { {1, ACTION_LEFT_ON}, {2, ACTION_LEFT_OFF}, {3, ACTION_LEFT_6_ON} },
    /* State 2: short double click (Turn off led one by one) */
    { {2, ACTION_LEFT_OFF}, {0, ACTION_NOP}, {4, ACTION_ALL_ON} },
    /* State 3: short click and long click (Turn on 6 led from the left) */
    { {3, ACTION_LEFT_6_ON}, {4, ACTION_ALL_ON}, {4, ACTION_ALL_ON} },
    /* State 4: long click and short click (Turn on all led) */
    { {4, ACTION_ALL_ON}, {5, ACTION_LEFT_4_ON}, {6, ACTION_ALL_OFF} },
    /* State 5: long double click (Turn off all led and return to initial state) */
    { {5, ACTION_LEFT_4_ON}, {6, ACTION_ALL_OFF}, {0, ACTION_RESET_TIMER} },
    /* State 6: Turn off all led and return to initial state*/
    { {0, ACTION_ALL_OFF}, {0, ACTION_RESET_TIMER}, {0, ACTION_RESET_TIMER} }
};
static int irq_number;
static int click_count = 0;

static const unsigned char led_array_map[8] = {
    GPIO_OUTPUT_LEFT_1,
    GPIO_OUTPUT_LEFT_2,
    GPIO_OUTPUT_LEFT_3,
    GPIO_OUTPUT_LEFT_4,
    GPIO_OUTPUT_RIGHT_1,
    GPIO_OUTPUT_RIGHT_2,
    GPIO_OUTPUT_RIGHT_3,
    GPIO_OUTPUT_RIGHT_4
};

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

static ssize_t toy_driver_read(struct file *filp, char __user *buf, \
size_t count, loff_t *offset)
{
    return BUF_SIZE;
}

ssize_t toy_driver_write(struct file *filp, const char __user *buf, \
size_t count, loff_t *offset)
{
    if(copy_from_user(kernel_write_buffer, buf, count)) {
        pr_err("write: error\n");
    }

    return count;


}

void set_bit_led(unsigned char led)
{
    int i;
    for(i = 0; i < 8; ++i) {
        pr_info("Setting GPIO %d to %d\n", led_array_map[i], \
        (led & (1 << i) ? 1 : 0));
        gpio_set_value(led_array_map[i], (led & (1 << i)) ? 1: 0);
    }
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = toy_driver_open,
    .release = toy_driver_close,
    .read = toy_driver_read,
    .write = toy_driver_write
};

static int register_gpio_output(int gpio_num)
{
    char gpio_name[80];

    snprintf(gpio_name, sizeof(gpio_name), "toy-gpio-%d", gpio_num);

    if(gpio_request(gpio_num, gpio_name)) {
        pr_info("Can not allocate GPIO\n");
        return -1;
    }

    if(gpio_direction_output(gpio_num, 0)) {
        pr_info("Can not set GPIO to output!\n");
        return -1;
    }

    return 0;
}

void test_led(void) 
{
    int i;
    for(i = 0; i < 8; ++i) {
        gpio_set_value(led_array_map[i], 1);
        msleep(500); 
        gpio_set_value(led_array_map[i], 0);
    }
}

void led_on_timer_callback(struct timer_list *t)
{
    int led_index = t - led_on_timers;

    gpio_set_value(led_array_map[led_index], 1);
    pr_info("LED %d ON\n", led_index + 1);
}

void start_led_on_sequence(void)
{
    int i;
    for(i = 0; i < 4; ++i) {
        timer_setup(&led_on_timers[i], led_on_timer_callback, 0);
        mod_timer(&led_on_timers[i], jiffies + msecs_to_jiffies(200 * i));
    }
}

void led_off_timer_callback(struct timer_list *t)
{
    int led_index = t - led_off_timers;
    
    gpio_set_value(led_array_map[led_index], 0);
    pr_info("LED %d OFF\n", led_index + 1);
    
}

void start_led_off_sequence(void)
{
    int i;
    for(i = 3; i >= 0; --i) {
        timer_setup(&led_off_timers[i], led_off_timer_callback, 0);
        mod_timer(&led_off_timers[i], jiffies + msecs_to_jiffies(200 * (3 - i)));
    }
}


void state_machine(int input);

// Function to handle timer expiration
static void click_timer_callback(struct timer_list *t) {
    if (click_count == 1) {
        pr_info("Single Click detected\n");
        state_machine(SW_ON);
    } else if (click_count == 2) {
        pr_info("Double Click detected\n");
        state_machine(T_OUT);
    }
    click_count = 0;
}

// ISR for the button press
static irqreturn_t button_isr(int irq, void *data) {
    click_count++;

    if (click_count == 1) {
        // First click detected, start the timer
        mod_timer(&click_timer, jiffies + msecs_to_jiffies(DOUBLE_CLICK_INTERVAL));
    } else if (click_count == 2) {
        // Second click detected, cancel the single-click and trigger double-click
        del_timer(&click_timer);
        click_timer_callback(&click_timer);
    }

    return IRQ_HANDLED;
}

void state_machine(int input)
{
    int i;
    static int state = 0;
    struct state_machine_element sm = state_machine_table[state][input];

    pr_info("Current state %d, Input: %d\n", state, input);
    state = sm.next_state;
    pr_info("Transition to state: %d, Action: %d\n", state, sm.action);

    //action
    switch(sm.action) {
        case ACTION_NOP:
            break;
        case ACTION_LEFT_ON:
            pr_info("LEFT LED ON");
            /*
            for(i = 0; i < 4; ++i) {
                if(!gpio_get_value(led_array_map[i])) {
                    gpio_set_value(led_array_map[i], 1);
                    msleep(200);
                    pr_info("GPIO %d set to %d\n", led_array_map[i], \
                    gpio_get_value(led_array_map[i]));
                    break;
                }
            }*/
           //msleep is not allowed on interrupt handler
           start_led_on_sequence(); // Turn on left 4 led one by one
            break;
        case ACTION_LEFT_OFF:
            pr_info("LEFT LED OFF");
            /*
            for(i = 3; i >= 0; --i) {
                if(gpio_get_value(led_array_map[i])) {
                    gpio_set_value(led_array_map[i], 0);
                    msleep(200);
                    pr_info("GPIO %d set to %d\n", led_array_map[i], \
                    gpio_get_value(led_array_map[i]));

                    break;
                }
            }*/
           //msleep is not allowed on interrupt handler
           start_led_off_sequence(); // Turn off left 4 led one by one
            break;
        case ACTION_ALL_ON:
            pr_info("ALL LED ON");
            set_bit_led(0xFF);
            break;
        case ACTION_ALL_OFF:
            pr_info("ALL LED OFF");
            set_bit_led(0x00);
            break;
        case ACTION_LEFT_4_ON:
            pr_info("LEFT 4 LED ON");
            for(i = 0; i < 4; ++i) {
                gpio_set_value(led_array_map[i], 1);
                pr_info("GPIO %d set to %d\n", led_array_map[i], \
                gpio_get_value(led_array_map[i]));
            }
            break;
        case ACTION_LEFT_6_ON:
            pr_info("LEFT 6 LED ON");
            for(i = 0; i < 6; ++i) {
                gpio_set_value(led_array_map[i], 1);
                pr_info("GPIO %d set to %d\n", led_array_map[i], \
                gpio_get_value(led_array_map[i]));
            }
            break;
        case ACTION_RESET_TIMER:
            pr_info("RESET");
            set_bit_led(0x00);
            state = 0;
            break;
    }

}

static int __init toy_driver_init(void) {
    int result = 0;

    if(alloc_chrdev_region(&toy_dev, 0, 1, DRIVER_NAME) < 0) {
        pr_info("Device Nr. could not be allocated!\n");
        return -1;
    }

    pr_info("allocated Major = %d Minor = %d\n", MAJOR(toy_dev), MINOR(toy_dev));
    if((toy_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
        pr_info("Device class not be created!\n");
        goto cerror;
    }

    if(device_create(toy_class, NULL, toy_dev, NULL, DRIVER_NAME) == NULL) {
        pr_info("Can not create device file!\n");
        goto device_error;
    }

    cdev_init(&toy_device, &fops);

    if(cdev_add(&toy_device, toy_dev, 1) == -1) {
        pr_info("Registering of device kernel failed!\n");
        goto reg_error;
    }

    register_gpio_output(GPIO_OUTPUT_LEFT_1);
    register_gpio_output(GPIO_OUTPUT_LEFT_2);
    register_gpio_output(GPIO_OUTPUT_LEFT_3);
    register_gpio_output(GPIO_OUTPUT_LEFT_4);
    register_gpio_output(GPIO_OUTPUT_RIGHT_1);
    register_gpio_output(GPIO_OUTPUT_RIGHT_2);
    register_gpio_output(GPIO_OUTPUT_RIGHT_3);
    register_gpio_output(GPIO_OUTPUT_RIGHT_4);

    if(gpio_request(GPIO_BUTTON, "toy-gpio-16")) {
        pr_info("Can not allocate GPIO 16 Switch \n");
        goto gpio_error;
    }
    
    if(gpio_direction_input(GPIO_BUTTON)) {
        pr_info("Can not set GPIO 16 to input!\n");
        goto gpio_error;
    }
    gpio_set_debounce(GPIO_BUTTON, 200);  // Optional: Debounce the button

    test_led();
    // Map GPIO to IRQ number
    irq_number = gpio_to_irq(GPIO_BUTTON);

    // Setup the ISR
    result = request_irq(irq_number, (irq_handler_t) button_isr, IRQF_TRIGGER_FALLING, "gpio_button_irq", NULL);
    if (result) {
        pr_info("Failed to request IRQ: %d\n", result);
        return result;
    }

    // Initialize the timer
    timer_setup(&click_timer, click_timer_callback, 0);

    pr_info("GPIO driver loaded successfully\n");
    return 0;

gpio_error:
    gpio_free(GPIO_BUTTON);
    gpio_free(GPIO_OUTPUT_LEFT_1);
    gpio_free(GPIO_OUTPUT_LEFT_2);
    gpio_free(GPIO_OUTPUT_LEFT_3);
    gpio_free(GPIO_OUTPUT_LEFT_4);
    gpio_free(GPIO_OUTPUT_RIGHT_1);
    gpio_free(GPIO_OUTPUT_RIGHT_2);
    gpio_free(GPIO_OUTPUT_RIGHT_3);
    gpio_free(GPIO_OUTPUT_RIGHT_4);

reg_error:
    device_destroy(toy_class, toy_dev);

device_error:
    class_destroy(toy_class);

cerror:
    unregister_chrdev_region(toy_dev, 1);

    return -1;
}

static void __exit toy_driver_exit(void) {
    del_timer(&click_timer);
    free_irq(irq_number, NULL);
    gpio_free(GPIO_BUTTON);
    gpio_free(GPIO_OUTPUT_LEFT_1);
    gpio_free(GPIO_OUTPUT_LEFT_2);
    gpio_free(GPIO_OUTPUT_LEFT_3);
    gpio_free(GPIO_OUTPUT_LEFT_4);
    gpio_free(GPIO_OUTPUT_RIGHT_1);
    gpio_free(GPIO_OUTPUT_RIGHT_2);
    gpio_free(GPIO_OUTPUT_RIGHT_3);
    gpio_free(GPIO_OUTPUT_RIGHT_4);
    cdev_del(&toy_device);
    device_destroy(toy_class, toy_dev);
    class_destroy(toy_class);
    unregister_chrdev_region(toy_dev, 1);
    pr_info("GPIO driver unloaded successfully\n");
}

module_init(toy_driver_init);
module_exit(toy_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GwonTaekJoon<kyuzu123123@gmail.com>");
MODULE_DESCRIPTION("toy simple io using state machine with Single and Double Click Detection");
MODULE_VERSION("1.0.0");

