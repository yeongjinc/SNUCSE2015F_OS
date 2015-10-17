#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"

static void syscall_handler (struct intr_frame *);

void is_valid_pointer(void *ptr);
void *convert_userp(void *ptr);
void get_argument(struct intr_frame *f, int *argument, int n);
struct file *get_file(int fd);

struct custom_file
{
	struct list_elem file_elem;
	int fd;
	struct file *f;
};

struct file *get_file(int fd)
{
	struct list *l = &thread_current()->file_list;
	struct list_elem *e;

	for(e = list_begin(l); e != list_end(l); e = list_next(e))
	{
		struct custom_file *cf = list_entry(e, struct custom_file, file_elem);
		if(fd == cf->fd)
			return cf->f;
	}
	return NULL;
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
	int nsyscall, ret, args[10];
	int *esp = (int *)f->esp;

	is_valid_pointer(esp);

	nsyscall = *(esp++);
	printf("syscall [%d]\n", nsyscall);	
	switch(nsyscall)
	{
    	case SYS_HALT:                   /* Halt the operating system. */
			halt();
			break;
    	case SYS_EXIT:                   /* Terminate this process. */
			get_argument(f, args, 1);
			exit(args[0]);
			break;
    	case SYS_EXEC:                   /* Start another process. */
    	case SYS_WAIT:                   /* Wait for a child process to die. */
    	case SYS_CREATE:                 /* Create a file. */
    	case SYS_REMOVE:                 /* Delete a file. */
    	case SYS_OPEN:                   /* Open a file. */
			get_argument(f, args, 1);
			args[0] = (int)convert_userp((void *)args[0]);
			f->eax = open((char *)args[0]);
			break;
    	case SYS_FILESIZE:               /* Obtain a file's size. */
    	case SYS_READ:                   /* Read from a file. */
    	case SYS_WRITE:                  /* Write to a file. */
    		get_argument(f, args, 3);
			args[1] = (int)convert_userp((void *)args[1]);
			f->eax = write(args[0], (const void *)args[1], (unsigned)args[2]);
			break;
		case SYS_SEEK:                   /* Change position in a file. */
    	case SYS_TELL:                   /* Report current position in a file. */
    	case SYS_CLOSE:                  /* Close a file. */
			break;
		default:
			thread_exit();	
	}

	f->eax = ret;
}

void is_valid_pointer(void *ptr)
{
	if(ptr == NULL)
		exit(1);

	if( ! is_user_vaddr(ptr))
		exit(1);

	if(ptr < (void *)0x08048000)
		exit(1);
}

void *convert_userp(void *ptr)
{
	is_valid_pointer(ptr);
	return pagedir_get_page(thread_current()->pagedir, ptr);
}

void get_argument(struct intr_frame *f, int *argument, int n)
{
	int i;
	for(i=0; i<n; i++)
	{
		int *p = (int *)f->esp + 1 + i;
		is_valid_pointer((void *)p);
		*(argument+i) = *p;
	}
}

void
halt (void)
{
	shutdown_power_off();
}

void
exit (int status)
{
	
}


pid_t exec (const char *file)
{
	return 0;
}

int 
wait (pid_t pid)
{
	return 0;
}

bool
create (const char *filename, unsigned initial_size)
{
	if(filename != NULL)
		return filesys_create(filename, initial_size);
	else
		return false;
}

bool
remove (const char *filename)
{
    if(filename != NULL)
        return filesys_remove(filename);
    else
        return false;
}

int
open (const char *filename)
{
	struct file *f = filesys_open(filename);
	if(f == NULL)
		return -1;

	int new_fd = thread_current()->current_max_fd + 1;
	thread_current()->current_max_fd += 1;

	struct custom_file *cf = malloc(sizeof(struct custom_file));
	cf->f = f;
	cf->fd = new_fd;
	list_push_back(&thread_current()->file_list, &cf->file_elem);
	
	return new_fd;
}

int
filesize (int fd)
{
	return 0;
}

int
read (int fd, void *buffer, unsigned length)
{
	return 0;
}

int
write (int fd, const void *buffer, unsigned length)
{
	if(fd == 1 || fd == 2)	// it means stdout or stderr
	{
		int i;
		// 100 chars per putbuf
		for(i=0; i<=length/100; i++)
		{
			int _len = (length - i*100) >= 100? 100 : (length - i*100);

			putbuf(&buffer[100*i], _len);
		}
		return length;
	}
	else
	{
		struct file *f = get_file(fd);
		if(f == NULL)
			return -1;

		int real_length = file_write(fd, buffer, length);
		return real_length;
	}
}

void
seek (int fd, unsigned position)
{
}

unsigned
tell (int fd)
{
	return 0;
}

void
close (int fd)
{
}
