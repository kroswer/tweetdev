#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>

struct tweet_device
{
	char tweet[120];
	struct semaphore sem;
} tweetdev;

struct cdev *micdev;
int major_number;
int ret;

dev_t dev_num;
struct class *cl;

#define NOM_DEV	"tweetdev"

int device_open(struct inode *inode, struct file *filp){
	if(down_interruptible(&tweetdev.sem) != 0){
		printk(KERN_ALERT "tweetdev: dispositivo ocupado");
		return -1;
	}
	printk(KERN_INFO "tweetdev: dispositivo abierto");
	return 0;
}

ssize_t device_read(struct file* filp, char* bufEntrada, size_t bufCount, loff_t* offset){
	printk(KERN_INFO "tweetdev: leyendo del dispositivo");
	ret = copy_to_user(bufEntrada, tweetdev.tweet, bufCount);
	return ret;
}

ssize_t device_write(struct file* filp, const char* bufEntrada, size_t bufCount, loff_t* offset){
	printk(KERN_INFO "tweetdev: escribiendo del dispositivo");
	ret = copy_from_user(tweetdev.tweet, bufEntrada, bufCount);
	return ret;
}

int device_close(struct inode *inode, struct file *filp){
	up(&tweetdev.sem);
	printk(KERN_INFO "tweetdev: dispositivo cerrado");
	return 0;
}

struct file_operations fops = {
	.owner = NOM_DEV,
	.open = device_open,
	.release = device_close,
	.write = device_write,
	.read = device_read
};

static int driver_entry(void){
	ret = alloc_chrdev_region(&dev_num,0,1,NOM_DEV);
	cl = class_create(NOM_DEV, "miclase");
	device_create(cl, NULL, dev_num, NULL, NOM_DEV);
	if (ret < 0)
	{
		printk(KERN_ALERT "tweetdev: imposible obtener major number");
		return ret;
	}

	micdev = cdev_alloc();
	micdev->ops = &fops;
	micdev->owner = NOM_DEV;
	ret = cdev_add(micdev, dev_num, 1);
	if (ret < 0)
	{
		printk(KERN_ALERT "tweetdev: imposible agregar dispositivo al kernel");
		return ret;
	}
	sema_init(&tweetdev.sem, 1);

	return 0;
}

static void driver_exit(void){
	cdev_del(micdev);
	unregister_chrdev_region(dev_num, 1);
	printk(KERN_ALERT "tweetdev: driver descargado");
}

module_init(driver_entry);
module_exit(driver_exit);
MODULE_LICENSE("GPL");
