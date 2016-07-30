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
#include <linux/sched.h>

struct tweet_device 
{
	char tweet[120];
	struct semaphore sem;
} tweetdev;

static dev_t num_dev; // Variable global para el primer dispositivo
static struct cdev c_dev; // Variable global para la estructura del dispositivo de caracter
static struct class *cl; // Variable global para la clase de dispositivo
int ret;

//Funciónes para abrir y cerrar el dispositivo

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

// Se lee el dispositivo
static ssize_t my_read(struct file *f, char __user *buf, size_t count, loff_t* ppos)
{
	printk(KERN_INFO "Driver: read()\n");
	
	char *hello_str = "Hello, world!\n";
	int len = strlen(hello_str);
	
	//strncpy(tweetdev.tweet, hello_str, len);
	// Si el mensaje es mayor que el buffer manda error
	if (count < len)
		return -EINVAL;
	//Si el puntero no esta al inicio del mensaje se sale de la función
	if (*ppos != 0)
		return 0;
	
	//Copia de la memoria del kernel a la memoria del usuario
	if (copy_to_user(buf, tweetdev.tweet, len))
		return -EINVAL;
	
	//schedule();
	printk(KERN_INFO "Read: buff %s", buf);
	*ppos = len;
	strncpy(tweetdev.tweet, "", 120);
	return len;
	/*ret = copy_to_user(buf, tweetdev.tweet, len);
	printk(KERN_INFO "ret: %d\n", ret);
	strncpy(tweetdev.tweet, "", 120); 
	return 0;*/
}

//Escribimos en el dispositivo
static ssize_t my_write(struct file *f, const char __user *buf, size_t count, loff_t *off)
{
	printk(KERN_INFO "Driver: write()\n");
	int len = strlen(tweetdev.tweet);
	//printk(KERN_INFO "antes de ifs\n");
	//Verifica que el mensaje quepa en el buffer
	if(len>count)
		return -EBUSY;
	//Forzamos a que el offset sea cero
	*off = 0;
	/*if (*off != 0)
		return 0;*/;
	//schedule();
	//Copiamos memoria de usuario a memoria del kernel
	if(copy_from_user(tweetdev.tweet, buf, count))
		return -EINVAL;
	//schedule();
	//printk(KERN_INFO "quiero vamos\n");
	
	//printk(KERN_INFO "Write: buff %s", buf);
	//*off = 0;
	return len;
}

//Las funciones definidas por el driver que ejecutan operaciones en el dispositivo
static struct file_operations pugs_fops =
{
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.read = my_read,
	.write = my_write
};

//Funciona que se ejecuta al cargar el driver
static int __init tweetdev_init(void) /* Constructor */
{
	int ret;
	struct device *dev_ret;

	printk(KERN_INFO "tweetdev cargado");
	//Registra un nuevo dispositivo de caracter y se le asigna un major_number por medio del kernel
	if ((ret = alloc_chrdev_region(&num_dev, 0, 1, "devtweet")) < 0)
	{
		return ret;
	}
	//Se crea una clase de dispositivo
	if (IS_ERR(cl = class_create(THIS_MODULE, "chardrv")))
	{
		unregister_chrdev_region(num_dev, 1);
		return PTR_ERR(cl);
	}
	//Se agrega al dev
	if (IS_ERR(dev_ret = device_create(cl, NULL, num_dev, NULL, "tweetdev")))
	{
		class_destroy(cl);
		unregister_chrdev_region(num_dev, 1);
		return PTR_ERR(dev_ret);
	}
        //Inicializa la estructura cdev
	cdev_init(&c_dev, &pugs_fops);
	if ((ret = cdev_add(&c_dev, num_dev, 1)) < 0)
	{
		device_destroy(cl, num_dev);
		class_destroy(cl);
		unregister_chrdev_region(num_dev, 1);
		return ret;
	}
	return 0;
}

//Funcion que se ejecuta al descargar el modulo
static void __exit tweetdev_exit(void) /* Destructor */
{
	cdev_del(&c_dev);
	device_destroy(cl, num_dev);
	class_destroy(cl);
	unregister_chrdev_region(num_dev, 1);
	printk(KERN_INFO "Quitando tweetdev");
}

module_init(tweetdev_init);
module_exit(tweetdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Equipo 3 Embbeded Linux");
MODULE_DESCRIPTION("tweetdev");

