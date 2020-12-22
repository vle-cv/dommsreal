#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#define BUFF_SIZE 200
#define STRED_SIZE 101
MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

DECLARE_WAIT_QUEUE_HEAD(readQ);
DECLARE_WAIT_QUEUE_HEAD(writeQ);
struct semaphore sem;


char stred[STRED_SIZE];
int pos = 0;
int i = 0;
int k = 0;
int endRead = 0;
int br = 0;

int stred_open(struct inode *pinode, struct file *pfile);
int stred_close(struct inode *pinode, struct file *pfile);
ssize_t stred_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t stred_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = stred_open,
	.read = stred_read,
	.write = stred_write,
	.release = stred_close,
};


int stred_open(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully opened stred\n");
		return 0;
}

int stred_close(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully closed stred\n");
		return 0;
}

ssize_t stred_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	int ret;
	char buff[BUFF_SIZE];
	long int len = 0;

	if (endRead){
		endRead = 0;
		pos = 0;
		printk(KERN_INFO "Procitano\n");
		return 0;
	}
	if(pos==0){
		if(down_interruptible(&sem))
			return -ERESTARTSYS;
		while(strlen(stred) == 0)
		{
			up(&sem);
			if(wait_event_interruptible(readQ,(strlen(stred)>0)))
				return -ERESTARTSYS;
			if(down_interruptible(&sem))
				return -ERESTARTSYS;
		}
	}	
	len = scnprintf(buff, BUFF_SIZE, "%c", stred[pos++]);
	ret = copy_to_user(buffer, buff, len);
	if(ret)
		return -EFAULT;
	if(pos==strlen(stred)){
		endRead=1;
		up(&sem);
		wake_up_interruptible(&writeQ);
	}

	return len;
}

ssize_t stred_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{

	char buff[BUFF_SIZE];
	char mod[6][10]={"string=", "clear", "shrink", "append=", "truncate=", "remove="};
	int value;
	int ret;
	char buff_in[BUFF_SIZE+9];
	char instruc[BUFF_SIZE];
	int brmod=10;
	int k=0;
	int size;
	char* ret_char;
	char help_str[STRED_SIZE];
	char *pom;
	 
	ret = copy_from_user(buff, buffer, length);
	
	if(ret)
		return -EFAULT;
	if(length > STRED_SIZE){
		printk(KERN_ERR "Prekoracen makismala broj karaktera\n");
		return -EFAULT;
	} 
	buff[length-1]='\0';

	if(down_interruptible(&sem))
		return -ERESTARTSYS;
	while(strlen(stred)==STRED_SIZE)
	{
		up(&sem);
		if(wait_event_interruptible(writeQ,(strlen(stred)<STRED_SIZE)))
			return -ERESTARTSYS;
		if(down_interruptible(&sem))
			return -ERESTARTSYS;
	}

	strcpy(buff_in, buff);
	
	for(i=0; i<6; i++){
		if(!strncmp(buff_in, mod[i], strlen(mod[i]))){//trazimo koji mod je prosledjen
			brmod=i;
			break;
		}
	}

	strcpy(instruc, buff_in+strlen(mod[brmod]));

	switch (brmod){
		case 0://unos stringa u stred

			if(strlen(instruc)>STRED_SIZE){
				printk(KERN_ERR "Velicina poslatog stringa je prevelika \n");
				return -EFAULT;
			}	    

		        strcpy(stred, instruc);

			printk(KERN_ERR "String upisan \n");
			break;

		case 1://clear

			for(i=0; i < STRED_SIZE; i++){
				stred[i]=0;
			}
			printk(KERN_INFO "Odradjen clear stringa\n");
			break;

		case 2: //shrink

			if(stred[0]==' '){
				for(i=1; i < STRED_SIZE; i++){
					if(stred[i]==' '){
						k++;
					}
					else break;
				}	
			}

			strcpy(stred, stred+k);

			for(i=strlen(stred)-1; i>=0; i--){
				if(stred[i]==' '){
					stred[i]=0;
				}
				else break;
			}	
			printk(KERN_ERR "Razmaci obrisani\n");
			break;	

		case 3://append

			while(strlen(stred)+strlen(instruc) >= STRED_SIZE){
				up(&sem);
		if(wait_event_interruptible(writeQ,(strlen(stred)+strlen(instruc) <= STRED_SIZE)))
					return -ERESTARTSYS;
				if(down_interruptible(&sem))
			
					return -ERESTARTSYS;
			}

			strcat(stred, instruc);
			printk(KERN_INFO "Odradjen append na string\n");
			break;

		case 4://truncate

			ret = !kstrtoint(instruc, 10 , &br);

			if(ret==0){
				printk(KERN_ERR "Nije prosledjena vrednost za truncate\n");
				up(&sem);
				return -EFAULT;
			}

			if(br > strlen(stred)){
				printk(KERN_ERR "Prevelika vrednost za truncate\n");
				up(&sem);
				return -EFAULT;
			}
			
			k=strlen(stred);

			while(br){
				br--;
				k--;
			}

			stred[k]='\0';

			printk(KERN_INFO "Truncatte odradjen \n");
			break;
			
		case 5://remove
			pom=stred;
			size=strlen(instruc);
			ret_char=strstr(stred, instruc);
			while(ret_char!=NULL){
				*ret_char='\0';
				strcpy(help_str, stred);
				pom=ret_char+size+1;
				strcat(help_str, pom);
				strcpy(stred, help_str);

				ret_char=strstr(stred, instruc);
			}

			for (i=strlen(stred); i>strlen(help_str); i--){
				stred[i]=0;
			}

			printk(KERN_INFO "Substring uspesno obrisan \n");	
			
			break;

		default:
			printk(KERN_ERR "Pogresan unos \n");
			up(&sem);
			return -EFAULT;
	
	
	
	}

	up(&sem);
	wake_up_interruptible(&readQ);	
		
	return length;
}

static int __init stred_init(void)
{
   int ret = 0;
	int i=0;
	
	sema_init(&sem,1);

	//Initialize array
	for (i=0; i < STRED_SIZE; i++)
		stred[i] = 0;

   ret = alloc_chrdev_region(&my_dev_id, 0, 1, "stred");
   if (ret){
      printk(KERN_ERR "failed to register char device\n");
      return ret;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create(THIS_MODULE, "stred_class");
   if (my_class == NULL){
      printk(KERN_ERR "failed to create class\n");
      goto fail_0;
   }
   printk(KERN_INFO "class created\n");
   
   my_device = device_create(my_class, NULL, my_dev_id, NULL, "stred");
   if (my_device == NULL){
      printk(KERN_ERR "failed to create device\n");
      goto fail_1;
   }
   printk(KERN_INFO "device created\n");

	my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1);
	if (ret)
	{
      printk(KERN_ERR "failed to add cdev\n");
		goto fail_2;
	}
   printk(KERN_INFO "cdev added\n");
   printk(KERN_INFO "Hello world\n");

   return 0;

   fail_2:
      device_destroy(my_class, my_dev_id);
   fail_1:
      class_destroy(my_class);
   fail_0:
      unregister_chrdev_region(my_dev_id, 1);
   return -1;
}

static void __exit stred_exit(void)
{
   cdev_del(my_cdev);
   device_destroy(my_class, my_dev_id);
   class_destroy(my_class);
   unregister_chrdev_region(my_dev_id,1);
   printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(stred_init);
module_exit(stred_exit);
