#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/include/asm-generic/cputime.h>


unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall2)(void);

asmlinkage long new_sys_cs3013_syscall2(struct processinfo info) {
    /*
     * A value of -1 signifies field did not have appropriate value available
     */
    struct processinfo kernel_info;
    // https://docs.huihoo.com/doxygen/linux/kernel/3.7/structtask__struct.html
    struct task_struct *current_task = get_current();

    kernel_info.state = current_task.state;
    kernel_info.pid = current_task.pid;
    kernel_info.parent_pid = current_task.parent.pid;
    kernel_info.youngest_child = -1;
    kernel_info.younger_sibling = -1;
    kernel_info.older_sibling = -1;
    kernel_info.uid = current_uid();
    //  1325     struct timespec real_start_time;    /* boot based time */
    kernel_info.start_time = (current_task.real_start_time.tv_sec * 1000000) +
            (current_task.real_start_time.tv_nsec / 1000);
    kernel_info.user_time = cputime_to_usecs(current_task.utime);
    kernel_info.sys_time = cputime_to_usecs(current_task.stime);
    kernel_info.cutime = -1;
    kernel_info.cstime = -1;

    /*
     * These values will be used to determine fields
     * youngest child
     * younger_sibling
     * older_sibling
     * cutime
     * cstime
     * of the processinfo object
     */
    long long current_youngest_child_time = 0;
    long long current_younger_sibling_time = 0;
    long long current_older_sibling_time = 0;

    /*
     * http://tuxthink.blogspot.com/2011/03/listforeach-and-listentry.html
     * Article above explains how to iterate the list_head structure
     * of current_task's children and siblings
     */

    list_for_each(list, &current_task.children){
        struct task_struct child_task = list_entry(list, struct task_struct, children);

        long long this_child_time = (child_task.real_start_time.tv_sec * 1000000) +
                (child_task.real_start_time.tv_nsec / 1000);

        if (kernel_info.cutime == -1 || current_youngest_child_time > this_child_time) {
            kernel_info.youngest_child = child_task.pid;
            current_youngest_child_time = this_child_time;
        }

        /* Make sure we don't have the -1 value if we do find children
         * Sometimes system/user time can be 0 so both values have to
         * be checked independently
         */
        if (kernel_info.cutime == -1)
            kernel_info.cutime += 1;
        if (kernel_info.cstime == -1)
            kernel_info.cstime += 1;

        kernel_info.cutime += cputime_to_usecs(child_task.utime);
        kernel_info.cstime += cputime_to_usecs(child_task.stime);
    }

    /*
     * We iterate the second list_head containing this processes siblings
     */

    list_for_each(list, &current_task.sibling){
        struct task_struct sibling_task = list_entry(list, struct task_struct, sibling);

        long long this_sibling_runtime = (sibling_task.real_start_time.tv_sec * 1000000) +
                (sibling_task.real_start_time.tv_nsec / 1000);

        if (kernel_info.younger_sibling == -1 || (kernel_info.start_time > this_sibling_runtime &&
            current_younger_sibling_time < this_sibling_runtime)) {
            kernel_info.younger_sibling = sibling_task.pid;
            current_younger_sibling_time = this_child_time;
        }
        if (kernel_info.older_sibling == -1 || (kernel_info.start_time < this_sibling_runtime &&
            current_older_sibling_time > this_sibling_runtime)) {
            kernel_info.older_sibling = sibling_task.pid;
            current_older_sibling_time = this_child_time;
        }
    }

    if (copy_to_user(info, &kinfo, sizeof kinfo))
        printk(KERN_INFO "Copying to user failed \n");

    return 0;
}

static unsigned long **find_sys_call_table(void) {
  unsigned long int offset = PAGE_OFFSET;
  unsigned long **sct;
  
  while (offset < ULLONG_MAX) {
    sct = (unsigned long **)offset;

    if (sct[__NR_close] == (unsigned long *) sys_close) {
      printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX",
                (unsigned long) sct);
      return sct;
    }
    
    offset += sizeof(void *);
  }
  
  return NULL;
}

static void disable_page_protection(void) {
  /*
    Control Register 0 (cr0) governs how the CPU operates.

    Bit #16, if set, prevents the CPU from writing to memory marked as
    read only. Well, our system call table meets that description.
    But, we can simply turn off this bit in cr0 to allow us to make
    changes. We read in the current value of the register (32 or 64
    bits wide), and AND that with a value where all bits are 0 except
    the 16th bit (using a negation operation), causing the write_cr0
    value to have the 16th bit cleared (with all other bits staying
    the same. We will thus be able to write to the protected memory.

    It's good to be the kernel!
   */
  write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
  /*
   See the above description for cr0. Here, we use an OR to set the 
   16th bit to re-enable write protection on the CPU.
  */
  write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
  /* Find the system call table */
  if(!(sys_call_table = find_sys_call_table())) {
    /* Well, that didn't work. 
       Cancel the module loading step. */
    return -1;
  }
  
  /* Store a copy of all the existing functions */
  ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];

  /* Replace the existing system calls */
  disable_page_protection();

  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;
  
  enable_page_protection();
  
  /* And indicate the load was successful */
  printk(KERN_INFO "Loaded interceptor!");

  return 0;
}

static void __exit interceptor_end(void) {
  /* If we don't know what the syscall table is, don't bother. */
  if(!sys_call_table)
    return;
  
  /* Revert all system calls to what they were before we began. */
  disable_page_protection();
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
  enable_page_protection();

  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);