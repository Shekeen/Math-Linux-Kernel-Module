#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#include "math.h"

#define MAJOR_NUMBER 77
#define DEVICE_NAME "math"
#define MAX_USERS 4

static int math_init(void);
static void math_exit(void);
static int math_open(struct inode*, struct file*);
static int math_release(struct inode*, struct file*);
static long math_ioctl(struct file*, unsigned int, unsigned long);

static atomic_t user_num;

static struct file_operations fops = {
  .open = math_open,
  .release = math_release,
  .unlocked_ioctl = math_ioctl
};

static int math_init(void)
{
  int err;

  atomic_set(&user_num, MAX_USERS);

  err = register_chrdev(MAJOR_NUMBER, DEVICE_NAME, &fops);
  if (err < 0) {
    pr_err("Device registration failed\n");
    return err;
  }

  pr_info("Module \"math\" was loaded\n");
  pr_info("Run \"mknod /dev/math c 77 0\"\n");
  return 0;
}

static void math_exit(void)
{
  unregister_chrdev(MAJOR_NUMBER, DEVICE_NAME);
  pr_info("Module \"math\" was unloaded\n");
}

static int math_open(struct inode *inode, struct file *file)
{
  pr_info("Opening /dev/math file\n");

  if (atomic_dec_return(&user_num) < 0) {
    pr_err("User quota exceeded\n");
    atomic_inc(&user_num);
    return -EBUSY;
  }

  return 0;
}

static int math_release(struct inode *inode, struct file *file)
{
  pr_info("Closing /dev/math file\n");

  atomic_inc(&user_num);

  return 0;
}

static long math_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
  int first_op, second_op, result;
  long long first_op_l, second_op_l, result_l;
  int __user *arg_ptr = (int __user *)arg;
  int err;

  switch(cmd) {

  case MATH_IOCTL_SQR:
    if ((err = get_user(first_op, arg_ptr)) < 0)
      return err;

    first_op_l = (long long)first_op;
    result_l = first_op_l * first_op_l;
    if (result_l > INT_MAX || result_l < INT_MIN)
      return -EINVAL;

    result = (int)result_l;
    if ((err = put_user(result, arg_ptr + 1)) < 0)
      return err;
    break;

  case MATH_IOCTL_NEG:
    if ((err = get_user(first_op, arg_ptr)) < 0)
      return err;

    if (first_op == INT_MIN)
      return -EINVAL;

    result = -first_op;
    if ((err = put_user(result, arg_ptr + 1)) < 0)
      return err;
    break;

  case MATH_IOCTL_ADD:
    if ((err = get_user(first_op, arg_ptr)) < 0 ||
        (err = get_user(second_op, arg_ptr + 1)) < 0)
    {
      return err;
    }

    first_op_l = (long long)first_op;
    second_op_l = (long long)second_op;
    result_l = first_op_l + second_op_l;
    if (result_l > INT_MAX || result_l < INT_MIN)
      return -EINVAL;

    result = (int)result_l;
    if ((err = put_user(result, arg_ptr + 2)) < 0)
      return err;
    break;

  case MATH_IOCTL_SUB:
    if ((err = get_user(first_op, arg_ptr)) < 0 ||
        (err = get_user(second_op, arg_ptr + 1)) < 0)
    {
      return err;
    }

    first_op_l = (long long)first_op;
    second_op_l = (long long)second_op;
    result_l = first_op_l - second_op_l;
    if (result_l > INT_MAX || result_l < INT_MIN)
      return -EINVAL;

    result = (int)result_l;
    if ((err = put_user(result, arg_ptr + 2)) < 0)
      return err;
    break;

  case MATH_IOCTL_MUL:
    if ((err = get_user(first_op, arg_ptr)) < 0 ||
        (err = get_user(second_op, arg_ptr + 1)) < 0)
    {
      return err;
    }

    first_op_l = (long long)first_op;
    second_op_l = (long long)second_op;
    result_l = first_op_l * second_op_l;
    if (result_l > INT_MAX || result_l < INT_MIN)
      return -EINVAL;

    result = (int)result_l;
    if ((err = put_user(result, arg_ptr + 2)) < 0)
      return err;
    break;

  case MATH_IOCTL_DIV:
    if ((err = get_user(first_op, arg_ptr)) < 0 ||
        (err = get_user(second_op, arg_ptr + 1)) < 0)
    {
      return err;
    }

    if (second_op == 0 ||
        first_op == INT_MIN && second_op == -1)
    {
      return -EINVAL;
    }

    result = first_op / second_op;
    if ((err = put_user(result, arg_ptr + 2)) < 0)
      return err;
    break;

  default:
    return -EINVAL;
    break;
  }

  return 0;
}

module_init(math_init);
module_exit(math_exit);
MODULE_AUTHOR("Anton Guryanov <guryanov91@gmail.com>");
MODULE_DESCRIPTION("Math module");
MODULE_LICENSE("GPL");
