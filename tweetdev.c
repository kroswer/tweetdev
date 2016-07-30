#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

struct tweet_device
{
	char tweet[120];
	struct semaphore sem;
} tweetdev;

static dev_t first; // Global variable for the first device number
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl; // Global variable for the device class
int ret;

static int my_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Driver: open()\n");
	return 0;
}
static int my_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Driver: close()\n");
	return 0;
}
static ssize_t my_read(struct file *f, char __user *buf, size_t count, loff_t* ppos)
{
	printk(KERN_INFO "Driver: read()\n");
	
	char *hello_str = "Hello, world!\n";
	int len = strlen(hello_str); /* Don't include the null byte. */
	/*
	 * We only support reading the whole string at once.
	 */
	//strncpy(tweetdev.tweet, hello_str, len);
	if (count < len)
		return -EINVAL;
	/*
	 * If file position is non-zero, then assume the string has
	 * been read and indicate there is no more data to be read.
	 */
	if (*ppos != 0)
		return 0;
	/*
	 * Besides copying the string to the user provided buffer,
	 * this function also checks that the user has permission to
	 * write to the buffer, that it is mapped, etc.
	 */
	if (copy_to_user(buf, tweetdev.tweet, len))
		return -EINVAL;
	/*
	 * Tell the user how much data we wrote.
	 */
	printk(KERN_INFO "Read: buff %s", buf);
	*ppos = len;
	return len;
	/*ret = copy_to_user(buf, tweetdev.tweet, len);
	printk(KERN_INFO "ret: %d\n", ret);
	strncpy(tweetdev.tweet, "", 120); 
	return 0;*/
}
static ssize_t my_write(struct file *f, const char __user *buf, size_t count, loff_t *off)
{
	printk(KERN_INFO "Driver: write()\n");
	int len = strlen(tweetdev.tweet);
	printk(KERN_INFO "antes de ifs\n");
	if(len>count)
		return -EINVAL;
	*off = 0;
	/*if (*off != 0)
		return 0;*/
	printk(KERN_INFO "quiero copiar\n");
	if(copy_from_user(tweetdev.tweet, buf, count))
		return -EINVAL;
	//printk(KERN_INFO "Write: buff %s", buf);
	//*off = 0;
	return len;
}

static struct file_operations pugs_fops =
{
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.read = my_read,
	.write = my_write
};

static int __init ofcd_init(void) /* Constructor */
{
	int ret;
	struct device *dev_ret;

	printk(KERN_INFO "Namaskar: ofcd registered");
	if ((ret = alloc_chrdev_region(&first, 0, 1, "Shweta")) < 0)
	{
		return ret;
	}
	if (IS_ERR(cl = class_create(THIS_MODULE, "chardrv")))
	{
		unregister_chrdev_region(first, 1);
		return PTR_ERR(cl);
	}
	if (IS_ERR(dev_ret = device_create(cl, NULL, first, NULL, "mynull")))
	{
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return PTR_ERR(dev_ret);
	}

	cdev_init(&c_dev, &pugs_fops);
	if ((ret = cdev_add(&c_dev, first, 1)) < 0)
	{
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return ret;
	}
	return 0;
}

static void __exit ofcd_exit(void) /* Destructor */
{
	cdev_del(&c_dev);
	device_destroy(cl, first);
	class_destroy(cl);
	unregister_chrdev_region(first, 1);
	printk(KERN_INFO "Alvida: ofcd unregistered");
}

module_init(ofcd_init);
module_exit(ofcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anil Kumar Pugalia <email@sarika-pugs.com>");
MODULE_DESCRIPTION("Our First Character Driver");

