#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/delay.h>

#define BUF_SIZE 1024
#define MAX_TIMEOUT_MSEC 2000

#define DRIVER_NAME "k_i2c_driver"
#define DRIVER_CLASS "k_i2c_class"

#define I2C_BUS_AVAILABLE 1
#define SLAVE_DEVICE_NAME "k_BMP280"    //BMP280 BOSCH SENSOR
#define BMP280_SLAVE_ADDRESS 0x76
static char kernel_write_buffer[BUF_SIZE];
static dev_t k_dev;
static struct class *k_class;
static struct cdev k_device;

static struct i2c_adapter *bmp_i2c_adapter = NULL;
static struct i2c_client *bmp280_i2c_client = NULL;

static struct hrtimer sensor_hrtimer;

static const struct i2c_device_id bmp_id[] = {
    {SLAVE_DEVICE_NAME, 0},
    {}
};

static struct i2c_driver bmp_driver = {
    .driver = {
        .name = SLAVE_DEVICE_NAME,
        .owner = THIS_MODULE}
};

static struct i2c_board_info bmp_i2c_board_info = {
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, BMP280_SLAVE_ADDRESS)
};

s32 dig_T1, dig_T2, dig_T3;

static struct workqueue_struct *k_workqueue;
static void workqueue_fn(struct work_struct *work);

static DECLARE_WORK(work, workqueue_fn);

s32 read_temperature(void)
{
    int var1;
    int var2;
    s32 raw_temp;
    s32 d1, d2, d3;

    d1 = i2c_smbus_read_byte_data(bmp280_i2c_client, 0xFA);
    d2 = i2c_smbus_read_byte_data(bmp280_i2c_client, 0xFB);
    d3 = i2c_smbus_read_byte_data(bmp280_i2c_client, 0xFC);

    raw_temp = ((d1 << 16) | (d2 << 8) | d3) >> 4;
    pr_info("raw_temp : %d\n", raw_temp);

    var1 = ((((raw_temp >> 3) - (dig_T1 << 1))) * (dig_T2)) >> 11;
    
    var2 = (((((raw_temp >> 4) - (dig_T1)) * ((raw_temp >> 4) - (dig_T1))) >> 12) * (dig_T3)) >> 14;

    return ((var1 + var2) * 5 + 128) >> 8;
}

static ssize_t k_driver_read(struct file *flip, char __user *buf, size_t count, loff_t *offset)
{
    char out_string[20];
    int temperature;

    temperature = read_temperature();
    snprintf(out_string, sizeof(out_string), "%d.%d\n", temperature / 100, temperature % 100);

    pr_info("%s\n", out_string);
    if(copy_to_user(buf, out_string, count)) {
        pr_err("read: error!\n");
    }

    return count;
}

static ssize_t k_driver_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
    if(copy_from_user(kernel_write_buffer, buf, count)) {
        pr_err("write: error\n");
    }
    pr_info("write: done\n");

    return count;
}

static int k_driver_open(struct inode *device_file, struct file *instance)
{
    pr_info("open\n");
    return 0;
}

static int k_driver_close(struct inode *device_file, struct file *instance)
{
    pr_info("close\n");
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = k_driver_open,
    .release = k_driver_close,
    .read = k_driver_read,
    .write = k_driver_write
};

struct temp_attr
{
    struct attribute attr;
    int value;
};

static struct temp_attr notify = {
    .attr.name = "notify",
    .attr.mode = 0644,
    .value = 0,
};

static struct temp_attr trigger = {
    .attr.name = "trigger",
    .attr.mode = 0644,
    .value = 0,
};

static struct attribute *temp_attrs[] = {
    &notify.attr,
    &trigger.attr,
    NULL
};

static ssize_t show(struct kobject *kobj, struct attribute *attr, char *buf)
{
    struct temp_attr *da = container_of(attr, struct temp_attr, attr);
    pr_info("hello: show called (%s)\n", da -> attr.name);
    return scnprintf(buf, PAGE_SIZE, "%d\n", da -> value);
}

static struct kobject *toy_kobj;

static ssize_t store(struct kobject *kobj, struct attribute *attr, \
                    const char *buf, size_t len)
{
    struct temp_attr *da = container_of(attr, struct temp_attr, attr);
    sscanf(buf, "%d", &(da -> value));
    pr_info("sysfs_notify store %s = %d\n", da -> attr.name, da -> value);

    if(strcmp(da -> attr.name, "notify") == 0) {
        notify.value = da -> value;
        sysfs_notify(toy_kobj, NULL, "notify");
    } else if(strcmp(da -> attr.name, "trigger") == 0) {
        trigger.value = da -> value;
        sysfs_notify(toy_kobj, NULL, "trigger");
    }
    return sizeof(int);
}

static struct sysfs_ops s_ops = {
    .show = show,
    .store = store,
};

static struct kobj_type k_type = {
    .sysfs_ops = &s_ops,
    .default_attrs = temp_attrs,
};

static void workqueue_fn(struct work_struct *work)
{
    char out_string[20];
    int temperature;

    temperature = read_temperature();

    snprintf(out_string, sizeof(out_string), "%d.%d\n", \
            temperature / 100, temperature % 100);
    
    pr_info("%s\n", out_string);

    notify.value = temperature;
    pr_info("Calling sysfs_notify()\n");
    sysfs_notify(toy_kobj, NULL, "notify");
    pr_info("sysfs_notify() called\n");

}

enum hrtimer_restart sensor_timer_callback(struct hrtimer *timer)
{
    queue_work(k_workqueue, &work);
    hrtimer_forward_now(timer, ms_to_ktime(MAX_TIMEOUT_MSEC));
    return HRTIMER_RESTART;
}

static int __init k_module_init(void)
{
    u8 id;

    /* be allocated node */
    if(alloc_chrdev_region(&k_dev, 0, 1, DRIVER_NAME) < 0) {
        pr_info("Device Nr. could not be allocated\n");
        return -1;
    }

    pr_info("allocated Major = %d Minor = %d \n", MAJOR(k_dev), MINOR(k_dev));

    /* create device class */
    if((k_class = class_create(THIS_MODULE, DRIVER_CLASS) == NULL)) {
        pr_info("Device calss can not be created!\n");
        goto cerror;
    }

    /* create device file*/
    if(device_create(k_class, NULL, k_dev, NULL, DRIVER_NAME) == NULL) {
        pr_info("Can not create device file!\n");
        goto device_error;
    }

    /* init character device file */
    cdev_init(&k_device, &fops);

    /* regitster for kernel */
    if(cdev_add(&k_device, k_dev, 1) == -1) {
        pr_info("Registering of device to kernel failed!\n");
        goto reg_error;
    }

    bmp_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);

    if(bmp_i2c_adapter != NULL) {
        bmp280_i2c_client = i2c_new_client_device(bmp_i2c_adapter, &bmp_i2c_board_info);
        if(bmp280_i2c_client != NULL) {
            if(i2c_add_driver(&bmp_driver)) {
                pr_info("Can't add driver...\n");
                goto reg_error;
            }
        }
        i2c_put_adapter(bmp_i2c_adapter);
    }

    id = i2c_smbus_read_byte_data(bmp280_i2c_client, 0xD0);
    pr_info("ID: 0x%x\n", id);

    dig_T1 = i2c_smbus_read_word_data(bmp280_i2c_client, 0x88);
    dig_T2 = i2c_smbus_read_word_data(bmp280_i2c_client, 0x8a);
    dig_T3 = i2c_smbus_read_word_data(bmp280_i2c_client, 0x8c);

    i2c_smbus_write_byte_data(bmp280_i2c_client, 0xf5, 5 << 5);
    i2c_smbus_write_byte_data(bmp280_i2c_client, 0xf4, ((5 << 5) | (5 << 2) | (3 << 0)));

    pr_info("sysfs: init\n");
    toy_kobj = kzalloc(sizeof(*toy_kobj), GFP_KERNEL);
    if(toy_kobj) {
        kobject_init(toy_kobj, &k_type);
        if(kobject_add(toy_kobj, NULL, "%s", "toy")) {
            pr_info("Toy: kobject_add() failed\n");
            kobject_put(toy_kobj);
            toy_kobj = NULL;
            goto reg_error;
        }
    }

    hrtimer_init(&sensor_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    sensor_hrtimer.function = &sensor_timer_callback;
    hrtimer_start(&sensor_hrtimer, ms_to_ktime(MAX_TIMEOUT_MSEC), HRTIMER_MODE_REL);

    k_workqueue = create_workqueue("k_wq");

    return 0;

reg_error:
    device_destroy(k_class, k_dev);
device_error:
    class_destroy(k_class);
cerror:
    unregister_chrdev_region(k_dev, 1);
    return -1;
}

static void __exit k_module_exit(void)
{
    int retval;

    destroy_workqueue(k_workqueue);

    retval = hrtimer_cancel(&sensor_hrtimer);

    if(retval) {
        pr_info("timer del...\n");
    }

    if(toy_kobj) {
        kobject_put(toy_kobj);
        kfree(toy_kobj);
    }

    i2c_unregister_device(bmp280_i2c_client);
    i2c_del_driver(&bmp_driver);
    cdev_del(&k_device);
    device_destroy(k_class, k_dev);
    class_destroy(k_class);
    unregister_chrdev_region(k_dev, 1);
    pr_info("exit\n");
}

module_init(k_module_init);
module_exit(k_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GwonTaekJoon<kyuzu123123@gmail.com");
MODULE_DESCRIPTION("bmp280 i2c");
MODULE_VERSION("1.0.0");




