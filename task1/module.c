#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

#include "telephone_directory.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("NikEvKuz kuznetsov.ne@phystech.edu");
MODULE_DESCRIPTION("Simple telephone directory");
MODULE_VERSION("0.1");


#define DEVICE_NAME "telephone_directory_device"
#define MAX_SIZE 1000
#define COMMAND_SIZE 4

#define GET_COMMAND "get "
#define ADD_COMMAND "add "
#define DEL_COMMAND "del "

#define DEVICE_MAGIC 'p'
#define GET_IOCTL _IOR(DEVICE_MAGIC, 1, char*)
#define ADD_IOCTL _IOW(DEVICE_MAGIC, 2, struct user_data*)
#define DEL_IOCTL _IOW(DEVICE_MAGIC, 3, char*)

enum {
	DEVICE_NOT_USED = 0,
	DEVICE_IS_USED = 1,
};

static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char __user*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char __user*, size_t, loff_t*);
static long device_ioctl(struct file* file, unsigned int command, unsigned long arg);

static struct file_operations chardev_fops = {
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = device_ioctl,
};
 

static int major;

static struct class* cls;
static struct device* dev;

static atomic_t is_open = ATOMIC_INIT(DEVICE_NOT_USED);

char output[MAX_SIZE];

static int __init device_init(void) {
	major = register_chrdev(0, DEVICE_NAME, &chardev_fops);
	if (major < 0) {
		printk(KERN_INFO "Device registering failed with %d\n", major);
		return major;
	}

	cls = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(cls)) {
		printk(KERN_INFO "Class creation failed\n");
		unregister_chrdev(major, DEVICE_NAME);
		return -1;
	}

	dev = device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
	if (IS_ERR(dev)) {
		printk(KERN_INFO "Device creation failed\n");
		class_destroy(cls);
		unregister_chrdev(major, DEVICE_NAME);
		return -1;	
	}
 
	printk(KERN_INFO "Device created on /dev/%s\n", DEVICE_NAME);
	return 0;
}

static void __exit device_exit(void) {
	delete_telephone_directory();

	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, DEVICE_NAME);
}

static int device_open(struct inode* inode, struct file* file) {
	if (atomic_cmpxchg(&is_open, DEVICE_NOT_USED, DEVICE_IS_USED)) {
		printk(KERN_INFO "Cannot open device: device is already used\n");
		return -EBUSY;
	}
	printk(KERN_INFO "Device opened\n");
	return 0;
}

static int device_release(struct inode* inode, struct file* file) {
	atomic_set(&is_open, DEVICE_NOT_USED);
	printk(KERN_INFO "Device closed\n");
	return 0;
}

static ssize_t device_read(struct file* file, char __user* user_buffer, size_t length, loff_t* offset) {
	printk(KERN_INFO "Device is reading\n");

	if (*offset > 0) {
		return 0;
	}

	if (copy_to_user(user_buffer, output, strlen(output)) == 0) {
		*offset += strlen(output);
		return strlen(output);
	}

	printk(KERN_INFO "Device reading failed\n");
	return -1;
}

static ssize_t device_write(struct file* file, const char __user* user_buffer, size_t length, loff_t* offset) {
	printk(KERN_INFO "Device is writing\n");

	char buffer[MAX_SIZE];
	memset(buffer, 0, MAX_SIZE);
	memset(output, 0, MAX_SIZE);

	copy_from_user(buffer, user_buffer, length);

	if (strncmp(buffer, GET_COMMAND, COMMAND_SIZE) == 0) {
		char surname[MAX_FIELD_SIZE];
		memset(surname, 0, MAX_FIELD_SIZE);
		user_data_t user_data;
		sscanf(buffer + COMMAND_SIZE, "%s", surname);

		if (telephone_directory_get_user(surname, strlen(surname), &user_data) != 0) {
			strcpy(output, "User not found\0");
			return -1;
		}

		sprintf(output, "%s %s %u %s %s", user_data.name, user_data.surname, user_data.age, user_data.telephone_number, user_data.email);
	} else if (strncmp(buffer, ADD_COMMAND, COMMAND_SIZE) == 0) {
		user_data_t user_data;
		sscanf(buffer + COMMAND_SIZE, "%s %s %u %s %s", user_data.name, user_data.surname, &user_data.age, user_data.telephone_number, user_data.email);

		if (telephone_directory_add_user(&user_data) != 0) {
			strcpy(output, "User is already exist\0");
			return -1;
		}

		strcpy(output, "User is successfully added\0");
	} else if (strncmp(buffer, DEL_COMMAND, COMMAND_SIZE) == 0) {
		char surname[MAX_FIELD_SIZE];
		memset(surname, 0, MAX_FIELD_SIZE);
		sscanf(buffer + COMMAND_SIZE, "%s", surname);

		if (telephone_directory_del_user(surname, strlen(surname)) != 0) {
			strcpy(output, "User not found\0");
			return -1;
		}

		strcpy(output, "User successfully deleted\0");
	} else {
		strcpy(output, "Unknown command\0");
		return -1;
	}

	return length;
}

static long device_ioctl(struct file* file, unsigned int command, unsigned long arg) {
	switch (command) {
		case GET_IOCTL:
			return telephone_directory_get_user((const char*)arg, sizeof((const char*)arg) / sizeof(((const char*)arg)[0]), (user_data_t*)arg);
		case ADD_IOCTL:
			return telephone_directory_add_user((user_data_t*)arg);
		case DEL_IOCTL:
			return telephone_directory_del_user((const char*)arg, sizeof((const char*)arg) / sizeof(((const char*)arg)[0]));
		default:
			return -1;
    }
}

module_init(device_init);
module_exit(device_exit);
