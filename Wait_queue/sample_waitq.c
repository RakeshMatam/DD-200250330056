#include<linux/module.h>
#include<linux/init.h>
#include<linux/cdev.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include<linux/circ_buf.h>
#include<linux/slab.h>
#include<linux/uaccess.h>
#include<linux/sched.h>
#include<linux/wait.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DD");

#define SIZE 32 // size should be of power of 2

static dev_t devnum;
static struct cdev _cdev;
static struct circ_buf cbuf; // Circular buffer
static wait_queue_head_t _wq; // wait_queue where the process waits for an event 
static int sample_open(struct inode *inodep, struct file *filep){
	printk("Sample Open Function\n");
	return 0;
}

static int sample_close(struct inode *inodep, struct file *filep){
        printk("Sample Close Function\n");
        return 0;
}

static ssize_t sample_read(struct file *filep , char __user *ubuff , size_t count ,loff_t * offset){
	
	int i,ret,mini;

	//wait event
	wait_event_interruptible(_wq,CIRC_CNT(cbuf.head,cbuf.tail,SIZE)>0);
	// We are waiting For the Data available to us, otherwise it will go on Sleep();
        // if CIRC_CNT ==0 indicates that there is no data in the Circular buffer
	// if CIRC_CNT > 0 indicates that there exists atleast 1 byte of data in the circular Buffer	

	mini = min(count,(size_t)CIRC_CNT(cbuf.head,cbuf.tail,SIZE));

	printk("Size from Userspace for Read:%d\n",(int)count);
    	for ( i = 0; i < mini; i++)
    	{
        	ret=copy_to_user(ubuff+i,cbuf.buf+cbuf.tail,1);
        	if(ret)
        	{
            		printk("Error copying to user");
            		return -EFAULT;
        	}
        	printk("Copied %c to user\n",cbuf.buf[cbuf.tail]);
        	cbuf.tail = (cbuf.tail+1) & (SIZE-1);
    	}

    return i;
}	

static ssize_t sample_write(struct file *filep , const char __user *ubuff , size_t count ,loff_t * offset){
        int ret,i;
	printk("Size sent from the Userspace : %d\n",(int)count);
	for(i=0;i<count;i++)
	{
		ret = copy_from_user(cbuf.buf+cbuf.head,ubuff+i,1);
		//copy_from_user(Destination,source,no.of bytes that are copying)

		if(ret)
		{
			printk("Error Copying\n");
			return -EFAULT;
		}
		printk("Copied from User: %c \t",cbuf.buf[cbuf.head]);
		cbuf.head = (cbuf.head + 1 ) & (SIZE -1);

	}
	return i;
	wake_up(&_wq); // Notified the wait Queue that now Data Is present To Read(Come out from Wait State)
}

struct file_operations fops={
		.open = sample_open,
		.read = sample_read,
		.write = sample_write,
		.release = sample_close,
};

static int __init sample_init(void){
	int ret;
	devnum = MKDEV(42,0);// used for constructing a device number
	ret = register_chrdev_region(devnum,1,"sample_dev");// Request the Kernel
	if(ret) // non zero means not Successfull
	{
		printk("Kernel did not grant us device Number\n");
		return ret;
	}
	// When Control comes here indicates the Kernel Grant the Device Number
	cdev_init(&_cdev,&fops); // binds your cdev structure with file operations Structure

	cbuf.buf = kmalloc(SIZE,GFP_KERNEL); // kmalloc allocates memory in kernel
	// GFP_KERNEL is the flag that tells kernel how to allocate the memory
	
	if(!cbuf.buf){
		printk("Memory is Not Allocated\n");
		unregister_chrdev_region(devnum,1);
		return -1;
	}
	// waitqueue_init
	
	init_waitqueue_head(&_wq);
	

	ret = cdev_add(&_cdev,devnum,1);// Device is "Live" now 
	if(ret)
	{
		printk("Unable to add cdev to Kernel\n");
		kfree(cbuf.buf);
		unregister_chrdev_region(devnum,1);
		return ret;
	}
	printk("Done Initialization\n");
	return 0;
}

static void __exit sample_exit(void){
	cdev_del(&_cdev);
	kfree(cbuf.buf);
	unregister_chrdev_region(devnum,1);
	printk("Exit\n");	
}

module_init(sample_init);
module_exit(sample_exit);


