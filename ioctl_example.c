#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>		/*This is for File operations */
#include <linux/uaccess.h>	/* Required for the copy to user function */
#include <linux/ioctl.h>
#include <linux/errno.h>
#include <linux/slab.h>	/*This is required for kmalloc*/
#include <linux/jiffies.h>

#include "ioctl_example.h"

int simple_open(struct inode *inode, struct file *filp);
int simple_release(struct inode *inode, struct file *filp);
ssize_t simple_read(struct file *filp, char __user *buff, size_t count, loff_t *offp);
ssize_t simple_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);
static long int simple_ioctl(struct file *file, unsigned cmd, unsigned long arg);

dev_t dev_num;
#define CHAR_DEVICE_NAME 	"ioctl-example"
static struct class *my_class;
static const char bufferSize = 8;	/* Bytes */

static struct file_operations ioctl_fops = {
	.owner = THIS_MODULE,
	.read = simple_read,
	.write = simple_write,
	.open = simple_open,
	.release = simple_release,
	.unlocked_ioctl = simple_ioctl
};

struct MyStruct {
	struct cdev DevInStruct;
	u8 *buff;
};
struct MyStruct *myStruct = NULL;

static int __init my_pdrv_probe (void) {
	int result;
	unsigned long j;
	
	j = jiffies;
	
	pr_err("Hello! device probe started = %lu\n", j);
	
	myStruct = kmalloc(sizeof(struct MyStruct), GFP_KERNEL);
	if (!myStruct) {
		pr_err("Problem in setting memory for mystruct\n");
        	return -ENOMEM; 
	}
	
	result = alloc_chrdev_region(&dev_num, 0, 1, CHAR_DEVICE_NAME);
	if (result){
		pr_err("DEvice allocation failed\n");
		return result;
	}
	pr_err("Major = %u & Minor = %u", MAJOR(dev_num), MINOR(dev_num));
	
	my_class = class_create(THIS_MODULE, CHAR_DEVICE_NAME);
	if (IS_ERR(my_class)) {
		pr_err("Error in creating class");
		return PTR_ERR(my_class);
	}
	
	if(device_create(my_class, NULL, dev_num, NULL, CHAR_DEVICE_NAME) == NULL) {
		printk("Can not create device file!\n");
		class_destroy(my_class);
		return -1;
	}
	
	cdev_init(&myStruct->DevInStruct, &ioctl_fops);
	myStruct->DevInStruct.owner = THIS_MODULE;
	
	result = cdev_add(&myStruct->DevInStruct, dev_num, 1);
	if (result){
		unregister_chrdev_region(dev_num, 1);
		pr_err("Error in adding device\n");
		return result;
	}
	
	pr_err("Hello! device probe process finished successfully = %lu\n", j);
	return 0;
}

static void __exit my_pdrv_remove(void) {
	unsigned long j;
	j = jiffies;
	pr_err("good bye reader! = %lu\n", j);
	
	cdev_del(&myStruct->DevInStruct);
	
	kfree(myStruct);
	
	device_destroy(my_class, dev_num);

	class_destroy(my_class);
	
	unregister_chrdev_region(dev_num, 1);
	
	pr_err("good bye reader!yawol = %lu\n", j);
}

static char answer = 48;

static long int simple_ioctl(struct file *file, unsigned cmd, unsigned long arg){
	switch(cmd){
		case WR_VALUE:
			if(copy_from_user(&answer, (char *) arg, sizeof(answer))) 
				printk("ioctl_example - Error copying data from user!\n");
			else
				printk("ioctl_example - Update the answer to %c\n", answer);
			break;
			
		case RD_VALUE:
			if(copy_to_user((char *) arg, &answer, sizeof(answer))) 
				printk("ioctl_example - Error copying data to user!\n");
			else
				printk("ioctl_example - The answer was copied!\n");
			break;
	}
	return 0;
}

int simple_open(struct inode *inode, struct file *filp)
{
	struct MyStruct *myDev;
	
	pr_err("Opening file");
	
	/* The Char driver that was created in 'probe' is now stored in filp
	so that it can be used in later calls of read and write etc. */
	myDev = container_of(inode->i_cdev, struct MyStruct, DevInStruct);
	filp->private_data = myDev;
	
	if (!myDev->buff) {
		myDev->buff = kmalloc(bufferSize, GFP_KERNEL);
		if (!myDev->buff) {
			pr_err("open/ENOMEM\n");
			return -ENOMEM;
		}
	}
	pr_err("File opened");
	return 0;
}

int simple_release(struct inode *inode, struct file *filp)
{
	struct MyStruct *myDev = filp->private_data;
	
	pr_err("Closing file");
	kfree(myDev->buff);
	pr_err("File closed");
	
	return 0;
}

ssize_t simple_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	struct MyStruct *myDev = filp->private_data;
	
	pr_err("Reading started");
	if (copy_to_user(buff, myDev->buff, bufferSize)) {
		pr_err("There was a problem in reading");
		return -EFAULT;
	}
	pr_err("Read was successful");
	return count;
}

ssize_t simple_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
	struct MyStruct *myDev = filp->private_data;
	
	pr_err("Writing started");
	if (copy_from_user(myDev->buff, buff, bufferSize)) {
		pr_err("There was a problem in reading");
		return -EFAULT;
	}
	pr_err("Write was successful");
	return count;
}

module_init(my_pdrv_probe);
module_exit(my_pdrv_remove);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("bhushan42");
MODULE_DESCRIPTION("Simple IOCTL driver");

