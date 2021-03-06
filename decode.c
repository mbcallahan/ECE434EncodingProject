/**
 * @file   decode.c
 * @author Matthew Callahan from Derek Molloy
 * @date   11 November 2021
 * @version 0.1
 * @brief   A module to decode a three-repeat code 
 * based on introductory code from Derek Molloy.
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
 */

#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>          // Required for the copy to user function


#define  DEVICE_NAME "UARTdecode"    ///< The device will appear at /dev/UARTdecode using this value
#define  CLASS_NAME  "dec"        ///< The device class -- this is a character device driver


MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Matthew Callahan");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A device to decode repeat code messages");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users
#define IN_BUFF_SIZE 769
static int    majorNumber;                  ///< Stores the device number -- determined automatically
static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
static char temp[IN_BUFF_SIZE]={0}; //buffer for converting down the decoding
static short  messageSize;              ///< Used to remember the size of the string stored
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  decodeClass  = NULL; ///< The device-driver class struct pointer
static struct device* decodeDevice = NULL; ///< The device-driver device struct pointer


// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init decodeInit(void){
   printk(KERN_INFO "Decode: Initializing the Decoding module\n");

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "Decode failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "Decode: registered correctly with major number %d\n", majorNumber);

   // Register the device class
   decodeClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(decodeClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(decodeClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "Decode: device class registered correctly\n");

   // Register the device driver
   decodeDevice = device_create(decodeClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(decodeDevice)){               // Clean up if there is an error
      class_destroy(decodeClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(decodeDevice);
   }
   printk(KERN_INFO "Decode: device class created correctly\n"); // Made it! device was initialized
   return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit decodeExit(void){   
   device_destroy(decodeClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(decodeClass);                          // unregister the device class
   class_destroy(decodeClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   printk(KERN_INFO "Decode: Goodbye from the LKM!\n");
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   numberOpens++;

   printk(KERN_INFO "Decode: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, message, messageSize);
   int sendLength=messageSize;
   messageSize=0;
   if (error_count==0){            // if true then have success
      printk(KERN_INFO "Decode: Sent %d characters to the user\n", messageSize);
      return (sendLength);  // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "Decode: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is tripple bytewise into the message[] array
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
  //don't overfill the buffer
  if(len>IN_BUFF_SIZE)
    len=IN_BUFF_SIZE;
  unsigned long ret;
  ret=copy_from_user(temp,buffer,len);
  temp[len-1]='\0';//ensure there is an end on the string
  short i=0;
  char first,second,third;//prealocate variables
  
  while(temp[i]){//go until null termination
    first=temp[i];
    second=temp[i+1];
    third=temp[i+2];
    message[i/3]=(first&second&third)|((~first)&second&third)|(first&(~second)&third)|(first&second&(~third));
    i+=3;
  }
  messageSize=i/3;//length is decreased by factor of three
  printk(KERN_INFO "Decode: prepared message from UART");
  return len;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "Decode: Device successfully closed\n");
   return 0;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(decodeInit);
module_exit(decodeExit);
